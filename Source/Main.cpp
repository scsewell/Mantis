#include "stdafx.h"
#include "Mantis.h"

#include "Device/Window/Window.h"
#include "Device/Graphics/Instance.h"
#include "Device/Graphics/PhysicalDevice.h"
#include "Device/Graphics/LogicalDevice.h"
#include "Device/Graphics/Surface.h"
#include <random>
#include <chrono>
#include <thread>

EASTL_EASTDC_API int EA::StdC::Vsnprintf(char8_t* EA_RESTRICT pDestination, size_t n, const char8_t* EA_RESTRICT pFormat, va_list arguments)
{
    return vsnprintf(pDestination, n, pFormat, arguments);
}

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return malloc(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return malloc(size);
}

namespace Mantis
{
    void main(int c, char* args[])
    {
        // report the build information
        Logger::Info(MANTIS_VERSION_TEXT);

        Window::Init();

        eastl::shared_ptr<Window> window = Window::Create();

        while (!window->IsClosed())
        {
            Window::Update();
            //window->SwapBuffers();
        }

        Window::Deinit();

        /*
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(0, 1000);

        const int amount = 100;

        std::vector<Vector2> vec;
        std::vector<bool> res;
        vec.reserve(amount);
        res.reserve(amount);

        for (size_t i = 0; i < amount; i++)
        {
            vec.push_back(Vector2(dist6(rng) / 100.f));
        }

        Vector2 v = Vector2::One + Vector2(1.f, 3.f);
        v /= 2.0f;

        // Record start time
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < 10000000; i++)
        {
            res.push_back(v == vec[i % amount]);
        }
        Logger::InfoF("Time: %u", std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::high_resolution_clock::now() - start)).count());

        res.clear();

        start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < 10000000; i++)
        {
            res.push_back(v.x != vec[i % amount].x || v.y != vec[i % amount].y);
        }
        Logger::InfoF("Time: %u", std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::high_resolution_clock::now() - start)).count());

        Logger::InfoF("%i", res[0]);
        Logger::InfoF("%i", v == Vector2(1.f, 1.f));
        Logger::InfoF("%i", v != Vector2(1.f, 1.f));
        Logger::InfoF("%i", Vector2::One == Vector2(1.f, 1.f));
        Logger::InfoF("%i", Vector2::One != Vector2(1.f, 1.f));
        Logger::Info(v.ToString());
        Logger::InfoF("%f", v.Length());
        Logger::InfoF("%s", v.Normalized().ToString());
        Logger::InfoF("%s", v.NormalizedFast().ToString());
        Logger::Info("Hello!!!!!");
        Logger::InfoF("asdfsadf [%i, %.6f] sdf ", 340, 0.1f);
        */
    }
}

int main(int c, char* args[])
{
    Mantis::main(c, args);
}
