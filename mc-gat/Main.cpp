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

bool outputImages(const Array<Image>& images, int cellSize, const String& savePath) {
	if (images.empty())
		return false;

	for (auto&& p : step(images[0].size / cellSize)) {
		Image out(cellSize, cellSize * images.size());

		for (int i : step(images.size())) {
			images[i].clipped(
				p * cellSize,
				Size(cellSize, cellSize)
			).overwrite(
				out,
				Point(0, i * cellSize)
			);
		}

		if (!out.savePNG(savePath + L'/' + ToString(p.x + p.y * images[0].size.x / cellSize) + L".png"))
			return false;
	}

	return true;
}

void Main() {
	Window::SetTitle(L"MC-GAT ver 0.41");
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

	int frameCount = 0;
	// プレビューでどのコマを表示しているか
	int previewFrame = 0;

	int cellSize = 32;

	while (System::Update()) {
		// ファイル読み込み (ボタン押下)
		if (gui.button(L"fileOpen").pushed) {
			asel::FileOpenResult res = asel::dialogOpenMultiFile(1024).value_or(asel::FileOpenResult{});
			std::sort(res.files.begin(), res.files.end());

			for (auto&& f : res.files)
				f = res.directory + f;

			if (!res.files.empty()) {
				fileDir = res.directory;

				if (loadImages(res.files, images, textures, Vec2(Window::Width() - gui.getRect().w, Window::Height()) * 0.9))
					previewFrame = 0;
			}
		}
		// ファイル読み込み (D&D)
		if (Dragdrop::HasItems()) {
			auto&& pathes = Dragdrop::GetFilePaths();

			std::sort(pathes.begin(), pathes.end());

			if (!pathes.empty()) {
				fileDir = FileSystem::ParentPath(pathes[0]);

				if (loadImages(pathes, images, textures, Vec2(Window::Width() - gui.getRect().w, Window::Height()) * 0.9))
					previewFrame = 0;
			}
		}

		// プレビューのFPS設定
		if (gui.slider(L"fpsSetter").hasChanged) {
			previewFPS = gui.slider(L"fpsSetter").valueInt;
			gui.text(L"fpsValue").text = ToString(previewFPS);
		}
		// プレビューの描画
		if (!images.empty()) {
			++frameCount;
			if (frameCount > 60 - previewFPS) {
				frameCount = 0;
				++previewFrame;
				if (previewFrame >= textures.size())
					previewFrame = 0;
			}

			textures[previewFrame].drawAt(Vec2(gui.getRect().w + Window::Width(), Window::Height()) / 2.0);
		}

		// 出力解像度の変更
		if (gui.textField(L"resolution").hasChanged) {
			auto&& field = gui.textField(L"resolution");

			// 数字以外をエスケープ
			field.setText(field.text.remove_if([](wchar c) { return !IsDigit(c); }));
			cellSize = Parse<int>(field.text);
		}

		// 出力
		if (gui.button(L"output").pushed) {
			if (images.empty())
				MessageBox::Show(L"出力する画像を選択してください。");
			else {
				if (outputImages(images, cellSize, fileDir + L"out"))
					MessageBox::Show(L"ファイルの出力が完了しました。");
				else
					MessageBox::Show(L"ファイルの出力に失敗しました。");
			}
		}
	}
}
