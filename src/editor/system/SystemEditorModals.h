// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "editor/Modal.h"

using ImGuiID = unsigned int;

class SystemPath;

namespace Editor {

	class EditorApp;
	class SystemEditor;

	class FileActionOpenModal : public Modal {
	public:
		FileActionOpenModal(EditorApp *app);

	protected:
		void Draw() override;
		void DrawInternal() override;
	};

	class UnsavedFileModal : public Modal {
	public:
		UnsavedFileModal(EditorApp *app);

		enum ResultType {
			Result_Cancel,
			Result_No,
			Result_Yes
		};

		ResultType Result() { return m_result; }

	protected:
		void DrawInternal() override;
		ResultType m_result = Result_Cancel;
	};

	class NewSystemModal : public Modal {
	public:
		NewSystemModal(EditorApp *app, SystemEditor *editor, SystemPath *path);

		void Draw() override;

	protected:
		void DrawInternal() override;

		SystemEditor *m_editor;
		SystemPath *m_path;
	};
}
