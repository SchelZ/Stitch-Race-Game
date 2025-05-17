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

#include "queuedConnectionManager.h"
#include "connectionWriter.h"
#include "netAddress.h"


#include "config.h"

const float move_speed = 10.0f;
bool key_map[256] = { false };
WindowFramework* window;

PT(ClockObject) globalClock = ClockObject::get_global_clock();

//PT(QueuedConnectionManager)  conn_mgr;
//PT(ConnectionWriter)		 conn_writer;
//PT(Connection)               connection;

void send_position(const LVector3& pos) {
	Datagram dg;
	dg.add_float32(pos.get_x());
	dg.add_float32(pos.get_y());
	dg.add_float32(pos.get_z());
	// false = do not require “reliable” ack
	//conn_writer->send(dg, connection, false);
}

void setCursorHidden(WindowFramework* window, const bool hide) {
	WindowProperties props;
	props.set_cursor_hidden(hide);
	if (hide)
		props.set_mouse_mode(WindowProperties::M_relative);
	else 
		props.set_mouse_mode(WindowProperties::M_absolute);
	window->get_graphics_window()->request_properties(props);
}

AsyncTask::DoneStatus update_task(GenericAsyncTask* task, void* data) {
	NodePath cam = window->get_camera_group();
	float dt = globalClock->get_dt();

	LVector3 move(0, 0, 0);
	if (key_map['w']) move += LVector3(0, 1, 0);
	if (key_map['s']) move += LVector3(0, -1, 0);
	if (key_map['a']) move += LVector3(-1, 0, 0);
	if (key_map['d']) move += LVector3(1, 0, 0);

	if (move.length_squared() > 0.0f) {
		move.normalize();
		cam.set_pos(cam, move * move_speed * dt); // move relative to camera direction
	}

	send_position(cam.get_pos());
	return AsyncTask::DS_cont;
}

void on_key_down(const Event* ev, void* data) { key_map[(intptr_t)data] = true; }

void on_key_up(const Event* ev, void* data) { key_map[(intptr_t)data] = false; }

int main(int argc, char* argv[]) {
	load_prc_file_data("", game_prc_settings);

	QueuedConnectionManager* conn_mgr = new QueuedConnectionManager();
	ConnectionWriter* conn_writer = new ConnectionWriter(conn_mgr, 0);
	PT(Connection)           connection;

	NetAddress server_addr;
	if (!server_addr.set_host("127.0.0.1", 5000)) {
		std::cerr << "Invalid address\n"; 
		return EXIT_FAILURE;
	}

	connection = conn_mgr->open_TCP_client_connection(server_addr, 1.0);
	if (!connection) {
		std::cerr << "Failed to connect to server";
		return EXIT_FAILURE;
	}


	PandaFramework framework;
	framework.open_framework(argc, argv);

	window = framework.open_window();
	if (!window) {
		std::cerr << "Failed to open window.\n";
		return EXIT_FAILURE;
	}

	window->enable_keyboard();
	window->get_camera_group().set_pos(0, -20, 5);

	Camera* camera = window->get_camera(0);
	PT(Lens) lens = camera->get_lens();
	lens->set_fov(80);

	setCursorHidden(window, true);

	NodePath scene = window->load_model(framework.get_models(), "models/environment");
	if (!scene.is_empty()) {
		scene.reparent_to(window->get_render());
		scene.set_scale(0.25f, 0.25f, 0.25f);
		scene.set_pos(-8, 42, 0);

	} else { std::cerr << "Failed to load model: " << scene << "\n"; }


	framework.define_key("w", "move forward", on_key_down, (void*)'w');
	framework.define_key("w-up", "stop forward", on_key_up, (void*)'w');

	framework.define_key("a", "move left", on_key_down, (void*)'a');
	framework.define_key("a-up", "stop left", on_key_up, (void*)'a');

	framework.define_key("s", "move back", on_key_down, (void*)'s');
	framework.define_key("s-up", "stop back", on_key_up, (void*)'s');

	framework.define_key("d", "move right", on_key_down, (void*)'d');
	framework.define_key("d-up", "stop right", on_key_up, (void*)'d');

	// Add update task
	framework.get_task_mgr().add(new GenericAsyncTask("UpdateTask", &update_task, nullptr));

	
	framework.main_loop();

	conn_mgr->close_connection(connection);
	delete conn_writer;
	delete conn_mgr;
	framework.close_framework();
	return EXIT_SUCCESS;
}