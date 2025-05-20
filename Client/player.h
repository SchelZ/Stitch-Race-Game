#pragma once

enum p_color : int {
    BLUE,
    PURPLE,
    RED,
    GREEN
};

class Player {
public:
    Player(const std::string& p_name, const int car_id, const p_color color_id, const NodePath& car_mesh)
        : p_name(p_name), color_id(color_id), car_mesh(car_mesh) { }

    ~Player() { car_mesh.detach_node(); }
    
    NodePath& get_car_mesh() { return car_mesh; }
    const std::string& get_name() const { return p_name; }
    p_color get_color_id() const { return color_id; }

private:
    std::string p_name;
    p_color     color_id;
    NodePath    car_mesh;
};
