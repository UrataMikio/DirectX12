#pragma once
#include <memory>

class Window;
class D3D12Manager;

void ClassCreate();

std::shared_ptr<Window> window;
std::shared_ptr<D3D12Manager> manager;