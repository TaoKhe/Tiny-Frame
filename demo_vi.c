/**
 * Ví dụ sử dụng TinyFrame cơ bản - Tiếng Việt
 * Basic TinyFrame usage example - Vietnamese
 *
 * File này minh họa cách sử dụng TinyFrame để giao tiếp giữa hai thiết bị.
 * This file demonstrates how to use TinyFrame for communication between two devices.
 */

#include "TinyFrame.h"
#include <stdio.h>
#include <string.h>

// Khai báo instance TinyFrame | TinyFrame instance declaration
TinyFrame *tf;

// Mảng buffer giả lập để test | Mock buffer array for testing
static uint8_t test_buffer[1000];
static uint32_t buffer_pos = 0;

/**
 * Hàm tiện ích để hiển thị buffer dạng hex dump đẹp mắt
 * Utility function to display buffer as nice hex dump
 */
void print_hex_dump(const char* title, const uint8_t* data, uint32_t len)
{
    printf("%s (%u bytes):\n", title, len);
    
    for (uint32_t i = 0; i < len; i++)
    {
        // In địa chỉ offset ở đầu mỗi hàng | Print offset address at the beginning of each line
        if (i % 32 == 0)
        {
            printf("%04X: ", i);
        }
        
        printf("%02X ", data[i]);
        
        // Xuống hàng sau mỗi 32 byte | New line after every 32 bytes
        if ((i + 1) % 32 == 0)
        {
            printf("\n");
        }
    }
    
    // Xuống hàng nếu hàng cuối không đủ 32 byte | New line if last line is not complete
    if (len % 32 != 0)
    {
        printf("\n");
    }
    printf("────────────────────────────────────────────────────────────────────\n");
}

/**
 * Hàm ghi dữ liệu - PHẢI được implement bởi người dùng
 * Write function - MUST be implemented by user
 *
 * Trong ứng dụng thực, đây sẽ là UART_Send(), SPI_Write(), v.v.
 * In real application, this would be UART_Send(), SPI_Write(), etc.
 */
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    printf("Gửi %u bytes:\n", len);
    
    for (uint32_t i = 0; i < len; i++)
    {
        // In địa chỉ offset ở đầu mỗi hàng | Print offset address at the beginning of each line
        if (i % 32 == 0)
        {
            printf("%04X: ", i);
        }
        
        printf("%02X ", buff[i]);
        
        // Xuống hàng sau mỗi 32 byte | New line after every 32 bytes
        if ((i + 1) % 32 == 0)
        {
            printf("\n");
        }
        
        // Trong demo này, chúng ta lưu vào buffer thay vì gửi qua hardware
        // In this demo, we store to buffer instead of sending via hardware
        if (buffer_pos < sizeof(test_buffer))
        {
            test_buffer[buffer_pos++] = buff[i];
        }
    }
    
    // Xuống hàng nếu hàng cuối không đủ 32 byte | New line if last line is not complete
    if (len % 32 != 0)
    {
        printf("\n");
    }
    printf("────────────────────────────────────────────────────────────────────\n");
}

/**
 * Listener cho loại thông điệp "Hello"
 * Listener for "Hello" message type
 */
TF_Result listener_hello(TinyFrame *tf, TF_Msg *msg)
{
    printf("📨 Nhận được thông điệp HELLO!\n");
    printf("   Type: 0x%02X\n", msg->type);
    printf("   Frame ID: 0x%02X\n", msg->frame_id);
    printf("   Độ dài: %d bytes\n", msg->len);
    
    // Hiển thị dữ liệu dạng text | Display data as text
    printf("   Dữ liệu (text): ");
    for (int i = 0; i < msg->len; i++)
    {
        printf("%c", msg->data[i]);
    }
    printf("\n");
    
    // Hiển thị dữ liệu dạng hex nếu dài | Display hex data if long
    if (msg->len > 32)
    {
        print_hex_dump("   Dữ liệu (hex)", msg->data, msg->len);
    }

    // Gửi phản hồi | Send response
    TF_Msg response;
    TF_ClearMsg(&response);
    response.type = 0x02;              // Response type
    response.frame_id = msg->frame_id; // Sử dụng cùng ID | Use same ID
    response.is_response = true;

    const char *reply = "Chào bạn!";
    response.data = (uint8_t *)reply;
    response.len = strlen(reply);

    TF_Respond(tf, &response);

    return TF_STAY; // Giữ listener | Keep listener
}

/**
 * Listener cho dữ liệu cảm biến
 * Listener for sensor data
 */
TF_Result listener_sensor(TinyFrame *tf, TF_Msg *msg)
{
    printf("🌡️  Nhận được dữ liệu cảm biến!\n");

    if (msg->len >= 4)
    {
        // Giả sử 4 byte đầu là nhiệt độ (float) | Assume first 4 bytes are temperature (float)
        float temperature;
        memcpy(&temperature, msg->data, sizeof(float));
        printf("   Nhiệt độ: %.2f°C\n", temperature);
    }

    return TF_STAY;
}

/**
 * Listener dự phòng - bắt tất cả thông điệp không được xử lý
 * Fallback listener - catches all unhandled messages
 */
TF_Result listener_fallback(TinyFrame *tf, TF_Msg *msg)
{
    printf("❓ Thông điệp không xác định\n");
    printf("   Type: 0x%02X\n", msg->type);
    printf("   Frame ID: 0x%02X\n", msg->frame_id);
    printf("   Độ dài: %d bytes\n", msg->len);
    
    if (msg->len > 0)
    {
        // Hiển thị dữ liệu dạng hex | Display data as hex
        char title[100];
        snprintf(title, sizeof(title), "   Dữ liệu không xác định");
        print_hex_dump(title, msg->data, msg->len);
    }
    
    return TF_STAY;
}

/**
 * Hàm demo gửi các loại thông điệp khác nhau
 * Demo function to send different types of messages
 */
void demo_send_messages(void)
{
    printf("\n=== DEMO GỬI THÔNG ĐIỆP ===\n\n");

    // 1. Gửi thông điệp Hello đơn giản | Send simple Hello message
    printf("1️⃣  Gửi thông điệp Hello...\n");
    const char *hello_msg = "Xin chào từ TinyFrame!";
    TF_SendSimple(tf, 0x01, (uint8_t *)hello_msg, strlen(hello_msg));

    // 2. Gửi dữ liệu cảm biến | Send sensor data
    printf("\n2️⃣  Gửi dữ liệu cảm biến...\n");
    float temperature = 25.6f;
    TF_SendSimple(tf, 0x10, (uint8_t *)&temperature, sizeof(temperature));

    // 3. Gửi với struct TF_Msg | Send using TF_Msg struct
    printf("\n3️⃣  Gửi bằng struct TF_Msg...\n");
    TF_Msg msg;
    TF_ClearMsg(&msg);
    msg.type = 0x20;
    const char *custom_data = "Dữ liệu tùy chỉnh";
    msg.data = (uint8_t *)custom_data;
    msg.len = strlen(custom_data);
    TF_Send(tf, &msg);

    // 4. Demo multipart frame | Multipart frame demo
    printf("\n4️⃣  Demo Multipart Frame...\n");
    
    // Tạo dữ liệu có độ dài chính xác 163 bytes | Create data with exact 163 bytes length
    char large_data[164]; // 163 + 1 cho null terminator
    const char *base_text = "Đây là dữ liệu lớn được chia thành nhiều phần để gửi qua TinyFrame. "
                           "This is large data split into multiple parts for sending via TinyFrame. "
                           "Multipart frames are very useful!";
    
    // Copy base text và pad thêm để đủ 163 bytes | Copy base text and pad to reach 163 bytes
    strncpy(large_data, base_text, sizeof(large_data) - 1);
    large_data[sizeof(large_data) - 1] = '\0';
    
    // Đảm bảo có đúng 163 bytes bằng cách pad thêm space và số | Ensure exactly 163 bytes by padding
    int current_len = strlen(large_data);
    for (int i = current_len; i < 163; i++) {
        large_data[i] = '0' + (i % 10); // Pad với các số 0-9
    }
    large_data[163] = '\0';

    uint32_t total_len = 163; // Cố định 163 bytes | Fixed 163 bytes
    printf("   Tổng độ dài payload: %u bytes (frame tổng sẽ lớn hơn do header+checksum)\n", total_len);
    TF_SendSimple_Multipart(tf, 0x30, total_len);

    // Gửi theo từng chunk 32 byte | Send in 32-byte chunks
    uint32_t sent = 0;
    
    while (sent < total_len)
    {
        uint32_t chunk_size = (total_len - sent) > 32 ? 32 : (total_len - sent);
        TF_Multipart_Payload(tf, (uint8_t *)(large_data + sent), chunk_size);
        sent += chunk_size;
        printf("   Đã gửi payload chunk: %u/%u bytes\n", sent, total_len);
    }

    TF_Multipart_Close(tf);
    printf("   ✅ Hoàn thành multipart frame\n");
}

/**
 * Hàm demo nhận và xử lý thông điệp
 * Demo function to receive and process messages
 */
void demo_receive_messages(void)
{
    printf("\n=== DEMO NHẬN THÔNG ĐIỆP ===\n\n");

    // Hiển thị dữ liệu thô đã nhận | Display raw received data
    print_hex_dump("📥 Dữ liệu thô nhận được", test_buffer, buffer_pos);

    // Giả lập việc nhận dữ liệu từ buffer | Simulate receiving data from buffer  
    printf("🔄 Xử lý từng byte qua TinyFrame parser...\n");
    
    for (uint32_t i = 0; i < buffer_pos; i++)
    {
        TF_AcceptChar(tf, test_buffer[i]);
    }

    // Reset buffer để demo tiếp theo | Reset buffer for next demo
    buffer_pos = 0;
    printf("✅ Hoàn thành xử lý tất cả dữ liệu nhận được!\n");
}

/**
 * Hàm chính - Demo đầy đủ
 * Main function - Complete demo
 */
int main(void)
{
    printf("🚀 TINYFRAME DEMO\n");
    printf("================================\n\n");

    // 1. Khởi tạo TinyFrame | Initialize TinyFrame
    printf("1️⃣  Khởi tạo TinyFrame...\n");
    tf = TF_Init(TF_MASTER);
    if (!tf)
    {
        printf("❌ Lỗi khởi tạo TinyFrame!\n");
        return -1;
    }
    printf("✅ TinyFrame đã sẵn sàng!\n");

    // 2. Đăng ký các listener | Register listeners
    printf("\n2️⃣  Đăng ký listeners...\n");
    TF_AddTypeListener(tf, 0x01, listener_hello);  // Hello messages
    TF_AddTypeListener(tf, 0x10, listener_sensor); // Sensor data
    TF_AddGenericListener(tf, listener_fallback);  // Fallback
    printf("✅ Đã đăng ký listeners!\n");

    // 3. Demo gửi thông điệp | Demo sending messages
    demo_send_messages();

    // 4. Demo nhận thông điệp | Demo receiving messages
    demo_receive_messages();

    // 5. Dọn dẹp | Cleanup
    printf("\n5️⃣  Dọn dẹp...\n");
    TF_DeInit(tf);
    printf("✅ Hoàn thành!\n");

    printf("\n🎉 Demo TinyFrame hoàn tất!\n");
    printf("   Xem README.md để biết thêm chi tiết.\n");

    return 0;
}

/**
 * Các hàm mutex (nếu cần) | Mutex functions (if needed)
 * Chỉ cần implement nếu TF_USE_MUTEX = 1
 * Only need to implement if TF_USE_MUTEX = 1
 */
#if TF_USE_MUTEX
bool TF_ClaimTx(TinyFrame *tf)
{
    // Trong demo này không cần mutex thực | No real mutex needed in this demo
    return true;
}

void TF_ReleaseTx(TinyFrame *tf)
{
    // Không cần làm gì | Nothing to do
}
#endif