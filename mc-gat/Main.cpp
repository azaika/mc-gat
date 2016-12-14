#include <Siv3D.hpp>
#include "../asel/Asel.h"

bool loadImages(const Array<FilePath>& pathes, Array<Image>& images, Array<Texture>& textures, const Vec2& textureRect) {
	if (pathes.empty())
		return false;

	images.clear();
	textures.clear();
	images.reserve(pathes.size());
	textures.reserve(pathes.size());

	Size beforeSize = Size::Zero;
	for (auto&& path : pathes) {
		Image img(path);

		if (!img) {
			MessageBox::Show(L"画像の読み込みに失敗しました。読み込みを中止します。");

			textures.clear();
			images.clear();

			return false;
		}
		if (!beforeSize.isZero() && beforeSize != img.size) {
			MessageBox::Show(L"画像のサイズが一致していません。読み込みを中止します。");

			textures.clear();
			images.clear();

			return false;
		}
		beforeSize = img.size;

		images.push_back(img);

		textures.emplace_back(
			img.scaled(
				Min(
					textureRect.x / img.size.x,
					textureRect.y / img.size.y
				),
				Interpolation::Linear
			)
		);
	}

	return true;
}

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

	Array<Image> images;
	Array<Texture> textures;
	String fileDir;

	while (System::Update()) {
		// ファイル読み込み (ボタン押下)
		if (gui.button(L"fileOpen").pushed) {
			asel::FileOpenResult res = asel::dialogOpenMultiFile(1024).value_or(asel::FileOpenResult{});
			std::sort(res.files.begin(), res.files.end());

			for (auto&& f : res.files)
				f = res.directory + f;

			if (!res.files.empty()) {
				fileDir = res.directory;

				loadImages(res.files, images, textures, Vec2(Window::Width() - gui.getRect().w, Window::Height()) * 0.9);
			}
		}
		// ファイル読み込み (D&D)
		if (Dragdrop::HasItems()) {
			auto&& pathes = Dragdrop::GetFilePaths();

			std::sort(pathes.begin(), pathes.end());

			if (!pathes.empty()) {
				fileDir = FileSystem::ParentPath(pathes[0]);

				loadImages(pathes, images, textures, Vec2(Window::Width() - gui.getRect().w, Window::Height()) * 0.9);
			}
		}
	}
}
