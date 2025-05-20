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

#include "network.h"
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
	std::unique_ptr<NetworkClient> _networkClient;
	std::unique_ptr<Player> _localPlayer;				// make local player


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
		_framework.get_task_mgr().add(new GenericAsyncTask("LoadLocalPlayer", &Game::loadModelWrapper, new LoadModelTaskData{ this, _window->get_render(), "models/panda", LVecBase3(0, 0, 0), 0.25f }));
		_networkClient = std::make_unique<NetworkClient>("127.0.0.1", 5000);

		return true;
	}

	void onConnection() {
		if (!_localPlayer || !_networkClient) return;

		NodePath& np = _localPlayer->get_car_mesh();

		LVector3 pos = np.get_pos();
		LVecBase3 hpr = np.get_hpr();
		LVector3 scale = np.get_scale();

		Datagram dg;
		dg.add_string(_localPlayer->get_name());
		dg.add_int32(static_cast<int>(_localPlayer->get_color_id()));

		dg.add_float32(pos.get_x());
		dg.add_float32(pos.get_y());
		dg.add_float32(pos.get_z());

		dg.add_float32(hpr.get_x());  // h = yaw
		dg.add_float32(hpr.get_y());  // p = pitch
		dg.add_float32(hpr.get_z());  // r = roll

		dg.add_float32(scale.get_x());
		dg.add_float32(scale.get_y());
		dg.add_float32(scale.get_z());

		_networkClient->onConnection(dg);
	}


	// Generic model loader
	static AsyncTask::DoneStatus loadModelWrapper(GenericAsyncTask* task, void* data) {
		std::unique_ptr<LoadModelTaskData> p(static_cast<LoadModelTaskData*>(data));
		NodePath node = p->self->_window->load_model(p->self->_framework.get_models(), p->modelPath);
		if (!node.is_empty()) {
			node.reparent_to(p->parent);
			node.set_scale(p->scale);
			node.set_pos(p->pos);
		} else { nout << "Failed to load model: " << p->modelPath << "\n"; }

		if (strcmp((p->modelPath).c_str(), "models/panda") == 0) {
			p->self->_localPlayer = std::make_unique<Player>("LocalPlayer", 0, p_color::BLUE, node);
			p->self->onConnection();
		}
		return AsyncTask::DS_done;
	}
};
