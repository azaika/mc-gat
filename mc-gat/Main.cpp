#include <Siv3D.hpp>
#include "../asel/Asel.h"

void Main() {
	Graphics::SetBackground(Palette::White);

	int previewFPS = 20;

	// GUIの初期化
	GUI gui(GUIStyle::Default);
	{
		gui.addln(GUIText::Create(L"プレビューのFPS"));
		gui.add(L"fpsSetter", GUISlider::Create(1.0, 60.0, static_cast<double>(previewFPS)));
		gui.addln(L"fpsValue", GUIText::Create(L"20"));
		gui.addln(GUIText::Create(L"テクスチャ解像度"));
		gui.add(GUIText::Create(L"x"));
		gui.addln(L"resolution", GUITextField::Create(4));
		gui.textField(L"resolution").setText(L"32");
		gui.addln(L"fileOpen", GUIButton::Create(L"ファイルを選択"));
		gui.addln(L"clear", GUIButton::Create(L"クリア"));
		gui.addln(L"output", GUIButton::Create(L"出力"));
	}

	while (System::Update()) {
	}
}
