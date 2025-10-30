//
// File cấu hình TinyFrame.
// TinyFrame configuration file.
//
// Đổi tên thành TF_Config.h
// Rename to TF_Config.h
//

#ifndef TF_CONFIG_H
#define TF_CONFIG_H

#include <stdint.h>
#include <stdio.h> // được sử dụng bởi macro TF_Error() được định nghĩa bên dưới | used by the TF_Error() macro defined below
// #include <esp8266.h> // khi sử dụng với esphttpd | when using with esphttpd

//----------------------------- ĐỊNH DẠNG FRAME | FRAME FORMAT ---------------------------------
// Định dạng có thể được điều chỉnh để phù hợp với nhu cầu ứng dụng cụ thể của bạn
// The format can be adjusted to fit your particular application needs

// Nếu kết nối đáng tin cậy, bạn có thể vô hiệu hóa byte SOF và checksum.
// Điều đó có thể tiết kiệm tới 9 byte overhead.
// If the connection is reliable, you can disable the SOF byte and checksums.
// That can save up to 9 bytes of overhead.

// ,-----+-----+-----+------+------------+- - - -+-------------,
// | SOF | ID  | LEN | TYPE | HEAD_CKSUM | DATA  | DATA_CKSUM  |
// | 0-1 | 1-4 | 1-4 | 1-4  | 0-4        | ...   | 0-4         | <- kích thước (bytes) | size (bytes)
// '-----+-----+-----+------+------------+- - - -+-------------'

// !!! CẢ HAI PEER PHẢI SỬ DỤNG CÙNG CÀI ĐẶT !!! | !!! BOTH PEERS MUST USE THE SAME SETTINGS !!!

// Điều chỉnh kích thước theo mong muốn (1,2,4) | Adjust sizes as desired (1,2,4)
#define TF_ID_BYTES 1   // Số byte cho ID frame | Number of bytes for frame ID
#define TF_LEN_BYTES 2  // Số byte cho độ dài payload | Number of bytes for payload length
#define TF_TYPE_BYTES 1 // Số byte cho loại frame | Number of bytes for frame type

// Loại checksum. Các tùy chọn: | Checksum type. Options:
//   TF_CKSUM_NONE, TF_CKSUM_XOR, TF_CKSUM_CRC8, TF_CKSUM_CRC16, TF_CKSUM_CRC32
//   TF_CKSUM_CUSTOM8, TF_CKSUM_CUSTOM16, TF_CKSUM_CUSTOM32
// Checksum tùy chỉnh yêu cầu bạn implement các hàm checksum (xem TinyFrame.h)
// Custom checksums require you to implement checksum functions (see TinyFrame.h)
#define TF_CKSUM_TYPE TF_CKSUM_CRC16

// Sử dụng byte SOF để đánh dấu bắt đầu frame | Use a SOF byte to mark the start of a frame
#define TF_USE_SOF_BYTE 1
// Giá trị của byte SOF (nếu TF_USE_SOF_BYTE == 1) | Value of the SOF byte (if TF_USE_SOF_BYTE == 1)
#define TF_SOF_BYTE 0x01

//----------------------- TƯƠNG THÍCH NỀN TẢNG | PLATFORM COMPATIBILITY ----------------------------

// được sử dụng cho bộ đếm tick timeout - nên đủ lớn cho tất cả timeout được sử dụng
// used for timeout tick counters - should be large enough for all used timeouts
typedef uint16_t TF_TICKS;

// được sử dụng trong vòng lặp lặp qua các listener
// used in loops iterating over listeners
typedef uint8_t TF_COUNT;

//----------------------------- CÁC THAM SỐ | PARAMETERS ----------------------------------

// Kích thước payload nhận tối đa (buffer tĩnh) | Maximum received payload size (static buffer)
// Payload lớn hơn sẽ bị từ chối. | Larger payloads will be rejected.
#define TF_MAX_PAYLOAD_RX 1024

// Kích thước buffer gửi. Payload lớn hơn sẽ được chia thành các phần và gửi
// trong nhiều lời gọi đến hàm write. Có thể giảm để giảm sử dụng RAM.
// Size of the sending buffer. Larger payloads will be split to pieces and sent
// in multiple calls to the write function. This can be lowered to reduce RAM usage.
#define TF_SENDBUF_LEN 128

// --- Số lượng Listener - xác định kích thước của bảng slot tĩnh ---
// --- Listener counts - determine sizes of the static slot tables ---

// Frame ID listeners (chờ phản hồi / thông điệp multi-part) | Frame ID listeners (wait for response / multi-part message)
#define TF_MAX_ID_LST 10
// Frame Type listeners (chờ frame với byte payload đầu tiên cụ thể) | Frame Type listeners (wait for frame with a specific first payload byte)
#define TF_MAX_TYPE_LST 10
// Generic listeners (dự phòng nếu không có listener nào khác bắt được) | Generic listeners (fallback if no other listener catches it)
#define TF_MAX_GEN_LST 5

// Timeout cho việc nhận & phân tích frame | Timeout for receiving & parsing a frame
// tick = số lần gọi TF_Tick() | ticks = number of calls to TF_Tick()
#define TF_PARSER_TIMEOUT_TICKS 10

// Có sử dụng mutex hay không - yêu cầu bạn implement TF_ClaimTx() và TF_ReleaseTx()
// Whether to use mutex - requires you to implement TF_ClaimTx() and TF_ReleaseTx()
#define TF_USE_MUTEX 1

// Hàm báo cáo lỗi. Để vô hiệu hóa debug, thay đổi thành define rỗng
// Error reporting function. To disable debug, change to empty define
#define TF_Error(format, ...) printf("[TF] " format "\n", ##__VA_ARGS__)

//------------------------- Kết thúc cấu hình người dùng | End of user config ------------------------------

#endif // TF_CONFIG_H
