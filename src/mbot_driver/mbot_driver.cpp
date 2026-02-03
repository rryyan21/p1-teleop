#include "mbot_driver/mbot_driver.hpp"

#include <vector>

using namespace rix::ipc;
using namespace rix::msg;

MBotDriver::MBotDriver(std::unique_ptr<interfaces::IO> input, std::unique_ptr<MBotBase> mbot)
    : input(std::move(input)), mbot(std::move(mbot)) {}

void MBotDriver::spin(std::unique_ptr<interfaces::Notification> notif) {
    std::vector<uint8_t> size_buffer(4);
    
    while (true) {
        if (notif->is_ready()) {
            geometry::Twist2DStamped stop_cmd;
            mbot->drive(stop_cmd);  
            return;
        }
        
        ssize_t bytes_read = input->read(size_buffer.data(), size_buffer.size());
        
        if (bytes_read == 0) {
            geometry::Twist2DStamped stop_cmd;
            mbot->drive(stop_cmd);  
            return;
        }
        
        if (bytes_read < 0) {
            continue;
        }
        
        standard::UInt32 size_msg;
        size_t offset = 0;
        if (!size_msg.deserialize(size_buffer.data(), size_buffer.size(), offset)) {
            continue;
        }
        
        uint32_t msg_size = size_msg.data;
        
        std::vector<uint8_t> msg_buffer(msg_size);
        bytes_read = input->read(msg_buffer.data(), msg_buffer.size());
        
        if (bytes_read == 0) {
            geometry::Twist2DStamped stop_cmd;
            mbot->drive(stop_cmd);  
            return;
        }
        
        if (bytes_read < 0) {
            continue;
        }
        
        geometry::Twist2DStamped twist_cmd;
        offset = 0;
        if (!twist_cmd.deserialize(msg_buffer.data(), msg_buffer.size(), offset)) {
            continue;
        }
        
        mbot->drive(twist_cmd);
    }
}
