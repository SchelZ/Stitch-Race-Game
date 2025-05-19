#pragma once

#include "pandaFramework.h"
#include "pandaSystem.h"
#include "windowFramework.h"
#include "load_prc_file.h"
#include "camera.h"
#include "windowProperties.h"
#include "perspectiveLens.h"
#include "nodePath.h"
#include "genericAsyncTask.h"
#include "keyboardButton.h"

#include "config.h"

class Menu {

};

class Game {
public:
	Game(int argc, char* argv[]): _framework() {
		load_prc_file_data("", game_prc_settings);
		_framework.open_framework(argc, argv);
	}

	~Game() { _framework.close_framework(); }

	bool initialize() { 
		if (!setupWindow()) return false; 
		return true;
	}

	void run() { _framework.main_loop(); }

private:
	// core Panda objects
	PandaFramework   _framework;
	WindowFramework* _window = nullptr;
	Camera*			 _cam = nullptr;

	struct LoadModelTaskData {
		Game*				self;
		NodePath            parent;
		std::string         modelPath;
		LVecBase3           pos;
		float               scale;
	};

	enum CURSOR_STAT : bool {
		hidden = true,
		visible = false
	};

	void setCursorHidden(WindowFramework* window, const bool hide) {
		WindowProperties props;
		props.set_cursor_hidden(hide);
		props.set_mouse_mode(hide ? WindowProperties::M_relative : WindowProperties::M_absolute);
		window->get_graphics_window()->request_properties(props);
	}

	bool setupWindow() {
		_window = _framework.open_window();
		if (!_window) {
			nout << "Failed to open window.\n";
			return false;
		}

		_window->enable_keyboard();
		_window->get_camera_group().set_pos(0, -20, 5);
		_cam = _window->get_camera(0);
		_cam->get_lens()->set_fov(80);

		setCursorHidden(_window, CURSOR_STAT::hidden);

		_framework.get_task_mgr().add(new GenericAsyncTask("LoadScene", &Game::loadModelWrapper, new LoadModelTaskData{ this, _window->get_render(), "models/environment", LVecBase3(-8, 42, 0), 0.25f }));
		return true;
	}


	// Generic load model
	static AsyncTask::DoneStatus loadModelWrapper(GenericAsyncTask* task, void* data) {
		LoadModelTaskData* p = static_cast<LoadModelTaskData*>(data);
		NodePath node = p->self->_window->load_model(p->self->_framework.get_models(), p->modelPath);
		if (!node.is_empty()) {
			node.reparent_to(p->parent);
			node.set_scale(p->scale);
			node.set_pos(p->pos);
		} else { nout << "Failed to load model: " << p->modelPath << "\n"; }
		delete p;
		return AsyncTask::DS_done;
	}
};
