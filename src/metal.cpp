#include "detail/metal.hpp"
#include <array>
#include <regex>
#include <string_view>

namespace darmok
{
    namespace MetalCompiler
    {
        bool supports(bgfx::RendererType::Enum renderer) noexcept
        {
            return renderer == bgfx::RendererType::Metal;
        }

        // Rename the Slang entry point to the name bgfx's Metal renderer looks for.
        static void renameEntryPoint(std::string& msl, std::string_view entryName)
        {
            // Match "entryName(" or "entryName (" (optional space before paren).
            std::regex pat(std::string(entryName) + "(\\s*\\()");
            msl = std::regex_replace(msl, pat, "xlatMtlMain$1", std::regex_constants::format_first_only);
        }

        // Remove Slang's _MatrixStorage_* wrapper structs and replace with native float4x4/float3x3.
        // bgfx reads matrix uniforms via Metal reflection expecting MTLDataTypeFloat4x4/Float3x3,
        // not MTLDataTypeStruct (which is what the wrapper produces).
        static void inlineMatrixWrappers(std::string& msl)
        {
            static const std::pair<std::string, std::string> kTypes[] = {
                { "_MatrixStorage_float4x4_ColMajornatural_0", "float4x4" },
                { "_MatrixStorage_float3x3_ColMajornatural_0", "float3x3" },
            };
            for (auto& [storageType, metalType] : kTypes)
            {
                // Remove struct definition (body has no nested braces)
                msl = std::regex_replace(msl,
                    std::regex("struct " + storageType + "\\s*\\{[^}]*\\};\\s*\n?"), "");
                // Replace all remaining type name occurrences
                msl = std::regex_replace(msl, std::regex("\\b" + storageType + "\\b"), metalType);
            }
            // Fix Slang's column-array field accesses: .data_0[ -> [ (float4x4), .data_2[ -> [ (float3x3)
            msl = std::regex_replace(msl, std::regex("\\.data_0\\["), "[");
            msl = std::regex_replace(msl, std::regex("\\.data_2\\["), "[");
        }

        // Remove Slang's _Array_natural_*_0 wrapper structs (used for float4x4 arrays like u_skinning).
        // Without this, Metal reflection reports MTLDataTypeStruct instead of MTLDataTypeArray, causing
        // bgfx to assert in convertMtlType(). Replace with C-style float4x4 VARNAME[N] so Metal
        // reports MTLDataTypeArray with element MTLDataTypeFloat4x4.
        // Fix Slang's (&expr)->fieldName[  ->  expr[ by scanning backward with paren depth.
        // Handles arbitrary nesting (e.g. (&(&ctx)->ptr->field)->data[i]).
        static void fixArrayFieldAccess(std::string& msl, const std::string& fieldName)
        {
            const std::string tag = ")->" + fieldName + "[";
            size_t pos = 0;
            while ((pos = msl.find(tag, pos)) != std::string::npos)
            {
                int depth = 1;
                size_t scan = pos;
                bool matched = false;
                while (scan > 0)
                {
                    --scan;
                    char c = msl[scan];
                    if (c == ')') ++depth;
                    else if (c == '(')
                    {
                        if (--depth == 0)
                        {
                            if (scan + 1 < msl.size() && msl[scan + 1] == '&')
                            {
                                msl.replace(pos, tag.size(), "[");
                                msl.erase(scan, 2); // remove '(&'
                                pos = scan;
                                matched = true;
                            }
                            break;
                        }
                    }
                }
                if (!matched) pos += tag.size();
            }
        }

        static void inlineArrayWrappers(std::string& msl)
        {
            // Match a struct _Array_* containing exactly one array<float4x4, int(N)> field.
            static const std::regex kStructPat(
                "struct (_Array_\\S+)\\s*\\{\\s*array<float4x4, int\\((\\d+)\\)> (\\w+);\\s*\\};"
            );
            std::smatch match;
            while (std::regex_search(msl, match, kStructPat))
            {
                std::string structName = match[1].str();
                std::string numStr     = match[2].str();
                std::string fieldName  = match[3].str();

                // Remove the struct definition (including optional trailing newline)
                size_t start = static_cast<size_t>(match.position());
                size_t len   = static_cast<size_t>(match.length());
                msl.erase(start, len);
                if (start < msl.size() && msl[start] == '\n')
                    msl.erase(start, 1);

                // Replace the wrapper type name with metal::array (temporarily)
                msl = std::regex_replace(msl,
                    std::regex("\\b" + structName + "\\b"),
                    "array<float4x4, int(" + numStr + ")>");

                // Fix Slang's (&expr)->fieldName[ -> expr[ (handles nested parens)
                fixArrayFieldAccess(msl, fieldName);
            }
            // Convert "array<float4x4, int(N)> VARNAME;" struct field declarations to C-style
            // "float4x4 VARNAME[N];" so Metal reflection reports MTLDataTypeArray not MTLDataTypeStruct.
            msl = std::regex_replace(msl,
                std::regex("array<float4x4, int\\((\\d+)\\)> (\\w+);"),
                "float4x4 $2[$1];");
        }

        // Rename the [[buffer(0)]] parameter to "_mtl_u" so bgfx's Metal renderer can find
        // the uniform constant buffer by name in pipeline reflection data.
        static void renameUniformBuffer(std::string& msl)
        {
            static const std::regex kBufferPat("(\\w+)\\s*\\[\\[buffer\\(0\\)\\]\\]");
            std::smatch match;
            if (std::regex_search(msl, match, kBufferPat))
            {
                std::string origName = match[1].str();
                if (origName != "_mtl_u")
                    msl = std::regex_replace(msl, std::regex("\\b" + origName + "\\b"), "_mtl_u");
            }
        }

        // Remove the "_0" suffix Slang adds to vertex attribute struct fields.
        // bgfx matches attribute names from Metal reflection against its canonical table
        // (e.g. "a_position"), so the mangled names must be stripped.
        static void demangleAttributes(std::string& msl)
        {
            static const std::array<std::string_view, 18> kAttribs = {
                "a_position", "a_normal", "a_tangent", "a_bitangent",
                "a_color0",   "a_color1", "a_color2",  "a_color3",
                "a_indices",  "a_weight",
                "a_texcoord0", "a_texcoord1", "a_texcoord2", "a_texcoord3",
                "a_texcoord4", "a_texcoord5", "a_texcoord6", "a_texcoord7",
            };
            for (auto attrib : kAttribs)
            {
                msl = std::regex_replace(msl,
                    std::regex(std::string(attrib) + "_0(?![a-zA-Z0-9_])"),
                    std::string(attrib));
            }
        }

        expected<std::string, std::string> postProcess(std::string msl, std::string_view entryName) noexcept
        {
            try
            {
                renameEntryPoint(msl, entryName);
                inlineMatrixWrappers(msl);
                inlineArrayWrappers(msl);
                renameUniformBuffer(msl);
                demangleAttributes(msl);
                return msl;
            }
            catch (const std::exception& e)
            {
                return unexpected<std::string>{ e.what() };
            }
        }
    }
}
