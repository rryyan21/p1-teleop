#include "teleop_keyboard/teleop_keyboard.hpp"
#include <cctype>
#include <vector>
#include "rix/msg/geometry/Twist2DStamped.hpp"
#include "rix/msg/standard/UInt32.hpp"
#include "rix/msg/serialization.hpp"
#include "rix/util/time.hpp"

using namespace rix::msg;

TeleopKeyboard::TeleopKeyboard(
    std::unique_ptr<rix::ipc::interfaces::IO> input,
    std::unique_ptr<rix::ipc::interfaces::IO> output,
    double linear_speed,
    double angular_speed)
    : input(std::move(input)),
      output(std::move(output)),
      linear_speed(linear_speed),
      angular_speed(angular_speed) {}

void TeleopKeyboard::spin(
    std::unique_ptr<rix::ipc::interfaces::Notification> notif) {
    uint32_t seq = 0;
    
    while (true) {
        if (notif->is_ready()) {
            return;
        }
        
        uint8_t char_buffer;
        ssize_t bytes_read = input->read(&char_buffer, 1);
        
        if (bytes_read == 0) {
            return;
        }
        
        if (bytes_read < 0) {
            continue;
        }
        
        char key = static_cast<char>(char_buffer);
        
        geometry::Twist2DStamped twist_cmd;
        bool valid_key = true;
        
        switch (key) {
            case 'W':
            case 'w':
                twist_cmd.twist.vx = linear_speed;
                break;
            case 'A':
            case 'a':
                twist_cmd.twist.vy = linear_speed;
                break;
            case 'S':
            case 's':
                twist_cmd.twist.vx = -linear_speed;
                break;
            case 'D':
            case 'd':
                twist_cmd.twist.vy = -linear_speed;
                break;
            case 'Q':
            case 'q':
                twist_cmd.twist.wz = angular_speed;
                break;
            case 'E':
            case 'e':
                twist_cmd.twist.wz = -angular_speed;
                break;
            case ' ':
                // Space sends zero twist (already initialized to 0)
                break;
            default:
                // Ignore invalid keys
                valid_key = false;
                break;
        }
        
        if (!valid_key) {
            continue;
        }
        
        twist_cmd.header.seq = seq++;
        twist_cmd.header.frame_id = "mbot";
        twist_cmd.header.stamp = rix::util::Time::now().to_msg();
        
        size_t msg_size = twist_cmd.size();
        
        standard::UInt32 size_msg;
        size_msg.data = static_cast<uint32_t>(msg_size);
        std::vector<uint8_t> size_buffer(4);
        size_t offset = 0;
        size_msg.serialize(size_buffer.data(), offset);
        
        ssize_t written = output->write(size_buffer.data(), size_buffer.size());
        if (written < 0) {
            continue;
        }
        
        std::vector<uint8_t> msg_buffer(msg_size);
        offset = 0;
        twist_cmd.serialize(msg_buffer.data(), offset);
        
        written = output->write(msg_buffer.data(), msg_buffer.size());
        if (written < 0) {
            continue;
        }
    }
}