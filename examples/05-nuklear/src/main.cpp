#include <darmok/app.hpp>
#include <darmok/nuklear.hpp>
#include <darmok/data.hpp>

namespace
{
	using namespace darmok;

	enum class NuklearAppOp
	{
		Easy,
		Hard
	};

	class NuklearApp : public App, public INuklearRenderer
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);
			_nuklear = addComponent<NuklearAppComponent>(*this);
			auto& ctx = _nuklear->getContext();
			auto font = _nuklear->loadFont("assets/NotoSans-Regular.ttf");
			nk_style_set_font(&ctx, font.ptr());

			struct nk_color table[NK_COLOR_COUNT];
			table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
			table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
			table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
			table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
			table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
			table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
			table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
			table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
			table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
			table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
			table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
			table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
			table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
			table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
			table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
			table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
			table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
			table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
			table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
			table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
			table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
			table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
			table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
			table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
			table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
			table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
			table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
			table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
			nk_style_from_table(&ctx, table);
		}

		void nuklearRender(nk_context& ctx)
		{
			if (nk_begin(&ctx, "Show", nk_rect(50, 50, 220, 220),
				NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_SCALABLE)) {

				nk_layout_row_static(&ctx, 30, 80, 1);
				if (nk_button_label(&ctx, "button")) {
					/* event handling */
				}

				/* fixed widget window ratio width */
				nk_layout_row_dynamic(&ctx, 30, 2);
				if (nk_option_label(&ctx, "easy", _op == NuklearAppOp::Easy))
				{
					_op = NuklearAppOp::Easy;
				}
				if (nk_option_label(&ctx, "hard", _op == NuklearAppOp::Hard))
				{
					_op = NuklearAppOp::Hard;
				}

				/* custom widget pixel width */
				nk_layout_row_begin(&ctx, NK_STATIC, 30, 2);
				{
					nk_layout_row_push(&ctx, 50);
					nk_label(&ctx, "Volume:", NK_TEXT_LEFT);
					nk_layout_row_push(&ctx, 110);
					nk_slider_float(&ctx, 0, &_value, 1.0f, 0.1f);
				}
				nk_layout_row_end(&ctx);

				nk_layout_row_dynamic(&ctx, 35, 1);

				int len = _text.size();
				_text.copy((char*)_textBuffer.ptr(), len);
				auto ptr = (char*)_textBuffer.ptr();
				nk_edit_string(&ctx, NK_EDIT_SIMPLE, ptr, &len, _textBuffer.size(), nullptr);
				_text = std::string_view(ptr, len);
			};

			nk_end(&ctx);
		}

	private:
		NuklearAppOp _op;
		float _value;
		Data _textBuffer = Data(255);
		std::string _text = "testing";
		OptionalRef<NuklearAppComponent> _nuklear;
	};
}

DARMOK_MAIN(NuklearApp);
