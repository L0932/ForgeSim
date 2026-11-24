#include "../engine/Assets/src/ImageDecodeQueue.hpp"
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
}
int main()
{
    using ForgeSim::Assets::Detail::ProcessImagesInOrder;
    try
    {
        std::mutex mutex;
        std::condition_variable ready;
        bool laterStarted = false;
        bool advanced = false;
        unsigned active = 0, peak = 0;
        std::vector<std::size_t> output;
        ProcessImagesInOrder(9, 4, [&](std::size_t index)
        {
            std::unique_lock lock(mutex);
            peak = std::max(peak, ++active);
            if (index == 0)
                advanced = ready.wait_for(lock, std::chrono::seconds(5), [&] { return laterStarted; });
            if (index == 4) { laterStarted = true; ready.notify_all(); }
            --active;
            return index;
        }, [&](std::size_t value) { output.push_back(value); });
        Require(advanced, "Queue must begin later work while the first image is still pending");
        Require(peak <= 4, "Must respect the four-worker bound");
        for (std::size_t i=0; i<output.size(); ++i) Require(output[i]==i, "Image indices must remain ordered");
        Require(output.size()==9, "Every image must be consumed once");

        output.clear();
        try
        {
            ProcessImagesInOrder(8, 4, [](std::size_t index)
            {
                if (index==2 || index==5) throw std::runtime_error(std::to_string(index));
                return index;
            }, [&](std::size_t value) { output.push_back(value); });
            Require(false, "Decoder failure must propagate");
        }
        catch (const std::runtime_error& error)
        {
            Require(std::string(error.what())=="2", "First source-order error must propagate");
        }
        Require(output==std::vector<std::size_t>{0,1}, "No consumption beyond first failed image");
        output.clear();
        ProcessImagesInOrder(3, 0, [](std::size_t index){return index;},
            [&](std::size_t value){output.push_back(value);});
        Require(output==std::vector<std::size_t>{0,1,2}, "Serial path must preserve order");
        ProcessImagesInOrder(0, 4, [](std::size_t){throw std::runtime_error("Unexpected decode");return 0;}, [](int){});
        std::cout << "Image queue scheduling, ordering, worker bounds and errors passed\n";
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n';return 1; }
}
