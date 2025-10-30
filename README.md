# TinyFrame - Thư viện Giao thức Truyền thông

## Giới thiệu

TinyFrame là một thư viện giao thức truyền thông nhẹ được thiết kế để truyền dữ liệu đáng tin cậy qua các kênh không đáng tin cậy như UART, SPI, hoặc các giao diện nối tiếp khác. Thư viện này đặc biệt hữu ích cho các hệ thống nhúng và microcontroller.

## Tính năng chính

- **Nhẹ và hiệu quả**: Footprint code nhỏ, phù hợp cho microcontroller
- **Đáng tin cậy**: Có checksum và cơ chế phát hiện lỗi
- **Linh hoạt**: Hỗ trợ nhiều loại checksum (CRC16, CRC32, XOR, v.v.)
- **Callback-based**: Hệ thống listener linh hoạt
- **Multipart frames**: Hỗ trợ gửi dữ liệu lớn theo từng phần
- **Thread-safe**: Hỗ trợ mutex (tùy chọn)

## Cấu trúc Frame

```
,-----+-----+-----+------+------------+- - - -+-------------,                
| SOF | ID  | LEN | TYPE | HEAD_CKSUM | DATA  | DATA_CKSUM  |                
| 0-1 | 1-4 | 1-4 | 1-4  | 0-4        | ...   | 0-4         |
'-----+-----+-----+------+------------+- - - -+-------------'
```

- **SOF**: Start of Frame (tùy chọn)
- **ID**: Định danh frame (1-4 byte)
- **LEN**: Độ dài payload (1-4 byte)
- **TYPE**: Loại thông điệp (1-4 byte)
- **HEAD_CKSUM**: Checksum header (0-4 byte)
- **DATA**: Dữ liệu payload
- **DATA_CKSUM**: Checksum dữ liệu (0-4 byte)

## Cách sử dụng cơ bản

### 1. Cấu hình

Sao chép `TF_Config.example.h` thành `TF_Config.h` và điều chỉnh các tham số:

```c
// Kích thước các trường
#define TF_ID_BYTES     1    // 1-4 byte
#define TF_LEN_BYTES    2    // 1-4 byte  
#define TF_TYPE_BYTES   1    // 1-4 byte

// Loại checksum
#define TF_CKSUM_TYPE TF_CKSUM_CRC16

// Buffer sizes
#define TF_MAX_PAYLOAD_RX 1024  // Kích thước payload nhận tối đa
#define TF_SENDBUF_LEN    128   // Kích thước buffer gửi
```

### 2. Khởi tạo

```c
#include "TinyFrame.h"

// Tạo instance
TinyFrame *tf = TF_Init(TF_MASTER); // hoặc TF_SLAVE

// Hoặc sử dụng static allocation
TinyFrame tf_instance;
TF_InitStatic(&tf_instance, TF_MASTER);
```

### 3. Implement hàm ghi dữ liệu

```c
// Hàm này PHẢI được implement bởi người dùng
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    // Gửi dữ liệu qua UART, SPI, hoặc interface khác
    for(uint32_t i = 0; i < len; i++) {
        uart_send_byte(buff[i]);
    }
}
```

### 4. Gửi dữ liệu

```c
// Gửi đơn giản
TF_SendSimple(tf, 0x01, data, data_len);

// Gửi với struct
TF_Msg msg;
TF_ClearMsg(&msg);
msg.type = 0x01;
msg.data = data;
msg.len = data_len;
TF_Send(tf, &msg);
```

### 5. Nhận dữ liệu

```c
// Đăng ký listener cho loại frame cụ thể
bool my_listener(TinyFrame *tf, TF_Msg *msg) {
    printf("Nhận được type %d, len %d\n", msg->type, msg->len);
    // Xử lý dữ liệu...
    return TF_STAY; // Giữ listener
}

TF_AddTypeListener(tf, 0x01, my_listener);

// Trong vòng lặp chính, feed dữ liệu vào TinyFrame
uint8_t rx_byte = uart_receive_byte();
TF_AcceptChar(tf, rx_byte);
```

### 6. Cập nhật định kỳ

```c
// Gọi định kỳ để xử lý timeout
void timer_callback() {
    TF_Tick(tf);
}
```

## Các loại Listener

### Type Listener
Lắng nghe frame với type cụ thể:
```c
TF_AddTypeListener(tf, 0x10, handle_sensor_data);
```

### ID Listener  
Lắng nghe phản hồi cho frame cụ thể (với timeout):
```c
TF_Query(tf, &msg, response_handler, timeout_handler, 1000);
```

### Generic Listener
Listener dự phòng, bắt tất cả frame không được xử lý:
```c
TF_AddGenericListener(tf, fallback_handler);
```

## Multipart Frames

Để gửi dữ liệu lớn:

```c
// Bắt đầu multipart frame
TF_SendSimple_Multipart(tf, 0x02, total_length);

// Gửi từng phần
TF_Multipart_Payload(tf, chunk1, chunk1_len);
TF_Multipart_Payload(tf, chunk2, chunk2_len);
// ...

// Kết thúc frame
TF_Multipart_Close(tf);
```

## Các loại Checksum hỗ trợ

- `TF_CKSUM_NONE`: Không checksum
- `TF_CKSUM_XOR`: XOR đơn giản  
- `TF_CKSUM_CRC8`: CRC8 Dallas/Maxim
- `TF_CKSUM_CRC16`: CRC16 (polynomial 0x8005)
- `TF_CKSUM_CRC32`: CRC32 (polynomial 0xedb88320)
- `TF_CKSUM_CUSTOM8/16/32`: Checksum tùy chỉnh

## Ví dụ hoàn chỉnh

Xem các file trong thư mục `demo/` để có ví dụ hoàn chỉnh:

- `demo/simple/`: Ví dụ cơ bản
- `demo/socket_demo/`: Demo với socket TCP
- `demo/simple_multipart/`: Ví dụ multipart frame

## Thread Safety

Nếu sử dụng trong môi trường đa luồng, bật mutex:

```c
#define TF_USE_MUTEX 1
```

Và implement các hàm mutex:

```c
bool TF_ClaimTx(TinyFrame *tf) {
    // Lấy mutex
    return true;
}

void TF_ReleaseTx(TinyFrame *tf) {
    // Giải phóng mutex  
}
```

## Lưu ý quan trọng

1. **Cả hai peer phải dùng cùng cấu hình** (kích thước trường, checksum, v.v.)
2. **Gọi `TF_Tick()` định kỳ** để xử lý timeout
3. **Implement `TF_WriteImpl()`** phù hợp với hardware
4. **Quản lý bộ nhớ** với các listener có timeout

## Giấy phép

MIT License - Xem file LICENSE để biết chi tiết.

## Tác giả

- Ondřej Hruška (c) 2017-2018
- URL gốc: https://github.com/MightyPork/TinyFrame