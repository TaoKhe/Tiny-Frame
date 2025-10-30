#ifndef TinyFrameH
#define TinyFrameH

/**
 * Thư viện giao thức TinyFrame
 * TinyFrame protocol library
 *
 * (c) Ondřej Hruška 2017-2018, MIT License
 * Không có trách nhiệm/bảo hành, tự do sử dụng cho mọi mục đích, phải giữ lại thông báo và giấy phép này
 * no liability/warranty, free for any use, must retain this notice & license
 *
 * URL gốc: https://github.com/MightyPork/TinyFrame
 * Upstream URL: https://github.com/MightyPork/TinyFrame
 */

#define TF_VERSION "2.3.0"

//---------------------------------------------------------------------------
#include <stdint.h>  // for uint8_t etc
#include <stdbool.h> // for bool
#include <stddef.h>  // for NULL
#include <string.h>  // for memset()
//---------------------------------------------------------------------------

// Loại checksum (0 = không có, 8 = ~XOR, 16 = CRC16 0x8005, 32 = CRC32)
// Checksum type (0 = none, 8 = ~XOR, 16 = CRC16 0x8005, 32 = CRC32)
#define TF_CKSUM_NONE 0     // không có checksum | no checksums
#define TF_CKSUM_XOR 8      // XOR đảo của tất cả byte payload | inverted xor of all payload bytes
#define TF_CKSUM_CRC8 9     // Dallas/Maxim CRC8 (1-wire)
#define TF_CKSUM_CRC16 16   // CRC16 với đa thức 0x8005 (x^16 + x^15 + x^2 + 1) | CRC16 with the polynomial 0x8005 (x^16 + x^15 + x^2 + 1)
#define TF_CKSUM_CRC32 32   // CRC32 với đa thức 0xedb88320 | CRC32 with the polynomial 0xedb88320
#define TF_CKSUM_CUSTOM8 1  // Checksum 8-bit tùy chỉnh | Custom 8-bit checksum
#define TF_CKSUM_CUSTOM16 2 // Checksum 16-bit tùy chỉnh | Custom 16-bit checksum
#define TF_CKSUM_CUSTOM32 3 // Checksum 32-bit tùy chỉnh | Custom 32-bit checksum

#include "TF_Config.h"

// region Xác định kiểu dữ liệu | Resolve data types

// Kiểu dữ liệu cho độ dài payload (1, 2 hoặc 4 byte)
// Data type for payload length (1, 2 or 4 bytes)
#if TF_LEN_BYTES == 1
typedef uint8_t TF_LEN;
#elif TF_LEN_BYTES == 2
typedef uint16_t TF_LEN;
#elif TF_LEN_BYTES == 4
typedef uint32_t TF_LEN;
#else
#error Giá trị không hợp lệ cho TF_LEN_BYTES, phải là 1, 2 hoặc 4 | Bad value of TF_LEN_BYTES, must be 1, 2 or 4
#endif

// Kiểu dữ liệu cho loại frame (1, 2 hoặc 4 byte)
// Data type for frame type (1, 2 or 4 bytes)
#if TF_TYPE_BYTES == 1
typedef uint8_t TF_TYPE;
#elif TF_TYPE_BYTES == 2
typedef uint16_t TF_TYPE;
#elif TF_TYPE_BYTES == 4
typedef uint32_t TF_TYPE;
#else
#error Giá trị không hợp lệ cho TF_TYPE_BYTES, phải là 1, 2 hoặc 4 | Bad value of TF_TYPE_BYTES, must be 1, 2 or 4
#endif

// Kiểu dữ liệu cho ID frame (1, 2 hoặc 4 byte)
// Data type for frame ID (1, 2 or 4 bytes)
#if TF_ID_BYTES == 1
typedef uint8_t TF_ID;
#elif TF_ID_BYTES == 2
typedef uint16_t TF_ID;
#elif TF_ID_BYTES == 4
typedef uint32_t TF_ID;
#else
#error Giá trị không hợp lệ cho TF_ID_BYTES, phải là 1, 2 hoặc 4 | Bad value of TF_ID_BYTES, must be 1, 2 or 4
#endif

// Kiểu dữ liệu cho checksum (phụ thuộc vào loại checksum được chọn)
// Data type for checksum (depends on the selected checksum type)
#if (TF_CKSUM_TYPE == TF_CKSUM_XOR) || (TF_CKSUM_TYPE == TF_CKSUM_NONE) || (TF_CKSUM_TYPE == TF_CKSUM_CUSTOM8) || (TF_CKSUM_TYPE == TF_CKSUM_CRC8)
// ~XOR (nếu là 0, vẫn dùng 1 byte - nhưng không sử dụng) | ~XOR (if 0, still use 1 byte - it won't be used)
typedef uint8_t TF_CKSUM;
#elif (TF_CKSUM_TYPE == TF_CKSUM_CRC16) || (TF_CKSUM_TYPE == TF_CKSUM_CUSTOM16)
// CRC16
typedef uint16_t TF_CKSUM;
#elif (TF_CKSUM_TYPE == TF_CKSUM_CRC32) || (TF_CKSUM_TYPE == TF_CKSUM_CUSTOM32)
// CRC32
typedef uint32_t TF_CKSUM;
#else
#error Giá trị không hợp lệ cho TF_CKSUM_TYPE | Bad value for TF_CKSUM_TYPE
#endif

// endregion

//---------------------------------------------------------------------------

/** Enum bit peer (dùng cho khởi tạo) | Peer bit enum (used for init) */
typedef enum
{
    TF_SLAVE = 0,  // Thiết bị slave | slave device
    TF_MASTER = 1, // Thiết bị master | master device
} TF_Peer;

/** Phản hồi từ các listener | Response from listeners */
typedef enum
{
    TF_NEXT = 0,  //!< Không được xử lý, để listener khác xử lý | Not handled, let other listeners handle it
    TF_STAY = 1,  //!< Đã xử lý, giữ listener | Handled, stay
    TF_RENEW = 2, //!< Đã xử lý, giữ listener và gia hạn - chỉ hữu ích với listener có timeout | Handled, stay, renew - useful only with listener timeout
    TF_CLOSE = 3, //!< Đã xử lý, xóa bỏ listener | Handled, remove self
} TF_Result;

/** Cấu trúc dữ liệu để gửi/nhận thông điệp | Data structure for sending / receiving messages */
typedef struct TF_Msg_
{
    TF_ID frame_id;   //!< ID thông điệp | message ID
    bool is_response; //!< cờ nội bộ, được đặt khi sử dụng hàm Respond. frame_id sẽ được giữ nguyên | internal flag, set when using the Respond function. frame_id is then kept unchanged.
    TF_TYPE type;     //!< loại thông điệp nhận được hoặc gửi đi | received or sent message type

    /**
     * Buffer của dữ liệu nhận được, hoặc dữ liệu để gửi.
     * Buffer of received data, or data to send.
     *
     * - Nếu (data == NULL) trong ID listener, nghĩa là listener đã hết thời gian chờ và
     *   người dùng nên giải phóng userdata và thực hiện các hành động thích hợp khác.
     * - If (data == NULL) in an ID listener, that means the listener timed out and
     *   the user should free any userdata and take other appropriate actions.
     *
     * - Nếu (data == NULL) và length khác không khi gửi frame, đó là bắt đầu một frame multi-part.
     *   Lời gọi này phải được theo sau bởi việc gửi payload và đóng frame.
     * - If (data == NULL) and length is not zero when sending a frame, that starts a multi-part frame.
     *   This call then must be followed by sending the payload and closing the frame.
     */
    const uint8_t *data;
    TF_LEN len; //!< độ dài của payload | length of the payload

    /**
     * Dữ liệu người dùng tùy chỉnh cho ID listener.
     * Custom user data for the ID listener.
     *
     * Dữ liệu này sẽ được lưu trữ trong slot listener và truyền đến ID callback
     * thông qua cùng các trường này trên thông điệp nhận được.
     * This data will be stored in the listener slot and passed to the ID callback
     * via those same fields on the received message.
     */
    void *userdata;
    void *userdata2;
} TF_Msg;

/**
 * Xóa sạch struct message
 * Clear message struct
 *
 * @param msg - thông điệp để xóa tại chỗ | message to clear in-place
 */
static inline void TF_ClearMsg(TF_Msg *msg)
{
    memset(msg, 0, sizeof(TF_Msg));
}

/** Typedef cho struct TinyFrame | TinyFrame struct typedef */
typedef struct TinyFrame_ TinyFrame;

/**
 * Callback cho TinyFrame Type Listener
 * TinyFrame Type Listener callback
 *
 * @param tf - instance của TinyFrame | instance
 * @param msg - thông điệp nhận được, userdata được điền vào bên trong object | the received message, userdata is populated inside the object
 * @return kết quả listener | listener result
 */
typedef TF_Result (*TF_Listener)(TinyFrame *tf, TF_Msg *msg);

/**
 * Callback cho TinyFrame Listener Timeout
 * TinyFrame Type Listener callback
 *
 * @param tf - instance của TinyFrame | instance
 * @param msg - thông điệp nhận được, userdata được điền vào bên trong object | the received message, userdata is populated inside the object
 * @return kết quả listener | listener result
 */
typedef TF_Result (*TF_Listener_Timeout)(TinyFrame *tf);

// ---------------------------------- KHỞI TẠO | INIT ------------------------------

/**
 * Khởi tạo engine TinyFrame.
 * Initialize the TinyFrame engine.
 * Cũng có thể được sử dụng để reset hoàn toàn (xóa tất cả listeners v.v.).
 * This can also be used to completely reset it (removing all listeners etc).
 *
 * Trường .userdata (hoặc .usertag) có thể được sử dụng để nhận dạng các instance khác nhau
 * trong hàm TF_WriteImpl() v.v. Đặt trường này sau khi init.
 * The field .userdata (or .usertag) can be used to identify different instances
 * in the TF_WriteImpl() function etc. Set this field after the init.
 *
 * Hàm này là wrapper của TF_InitStatic, gọi malloc() để lấy instance.
 * This function is a wrapper around TF_InitStatic that calls malloc() to obtain
 * the instance.
 *
 * @param peer_bit - peer bit để sử dụng cho bản thân | peer bit to use for self
 * @return TF instance hoặc NULL | TF instance or NULL
 */
TinyFrame *TF_Init(TF_Peer peer_bit);

/**
 * Khởi tạo engine TinyFrame sử dụng instance struct được cấp phát tĩnh.
 * Initialize the TinyFrame engine using a statically allocated instance struct.
 *
 * Trường .userdata / .usertag được bảo tồn khi TF_InitStatic được gọi.
 * The .userdata / .usertag field is preserved when TF_InitStatic is called.
 *
 * @param tf - instance
 * @param peer_bit - peer bit để sử dụng cho bản thân | peer bit to use for self
 * @return thành công | success
 */
bool TF_InitStatic(TinyFrame *tf, TF_Peer peer_bit);

/**
 * Hủy khởi tạo TF instance được cấp phát động
 * De-init the dynamically allocated TF instance
 *
 * @param tf - instance
 */
void TF_DeInit(TinyFrame *tf);

// ---------------------------------- CÁC HÀM API | API CALLS --------------------------------------

/**
 * Nhận các byte đến và phân tích frame
 * Accept incoming bytes & parse frames
 *
 * @param tf - instance
 * @param buffer - buffer byte để xử lý | byte buffer to process
 * @param count - số byte trong buffer | nr of bytes in the buffer
 */
void TF_Accept(TinyFrame *tf, const uint8_t *buffer, uint32_t count);

/**
 * Nhận một byte đến
 * Accept a single incoming byte
 *
 * @param tf - instance
 * @param c - ký tự nhận được | a received char
 */
void TF_AcceptChar(TinyFrame *tf, uint8_t c);

/**
 * Hàm này nên được gọi định kỳ.
 * This function should be called periodically.
 * Cơ sở thời gian được sử dụng để timeout các frame không hoàn chỉnh trong parser và
 * tự động reset nó.
 * The time base is used to time-out partial frames in the parser and
 * automatically reset it.
 * Nó cũng được sử dụng để hết hạn ID listeners nếu timeout được đặt khi đăng ký chúng.
 * It's also used to expire ID listeners if a timeout is set when registering them.
 *
 * Một nơi phổ biến để gọi từ đó là SysTick handler.
 * A common place to call this from is the SysTick handler.
 *
 * @param tf - instance
 */
void TF_Tick(TinyFrame *tf);

/**
 * Reset state machine của frame parser.
 * Reset the frame parser state machine.
 * Điều này không ảnh hưởng đến các listener đã đăng ký.
 * This does not affect registered listeners.
 *
 * @param tf - instance
 */
void TF_ResetParser(TinyFrame *tf);

// ---------------------------- CÁC LISTENER THÔNG ĐIỆP | MESSAGE LISTENERS -------------------------------

/**
 * Đăng ký một frame ID listener.
 * Register a frame ID listener.
 *
 * @param tf - instance
 * @param msg - thông điệp (chứa frame_id và userdata) | message (contains frame_id and userdata)
 * @param cb - callback
 * @param ftimeout - callback timeout
 * @param timeout - timeout tính bằng tick để tự động xóa listener (0 = giữ mãi mãi) | timeout in ticks to auto-remove the listener (0 = keep forever)
 * @return thành công | success
 */
bool TF_AddIdListener(TinyFrame *tf, TF_Msg *msg, TF_Listener cb, TF_Listener_Timeout ftimeout, TF_TICKS timeout);

/**
 * Xóa một listener theo message ID mà nó đã đăng ký
 * Remove a listener by the message ID it's registered for
 *
 * @param tf - instance
 * @param frame_id - frame mà chúng ta đang lắng nghe | the frame we're listening for
 */
bool TF_RemoveIdListener(TinyFrame *tf, TF_ID frame_id);

/**
 * Đăng ký một frame type listener.
 * Register a frame type listener.
 *
 * @param tf - instance
 * @param frame_type - loại frame để lắng nghe | frame type to listen for
 * @param cb - callback
 * @return thành công | success
 */
bool TF_AddTypeListener(TinyFrame *tf, TF_TYPE frame_type, TF_Listener cb);

/**
 * Xóa một listener theo type.
 * Remove a listener by type.
 *
 * @param tf - instance
 * @param type - type mà nó đã đăng ký | the type it's registered for
 */
bool TF_RemoveTypeListener(TinyFrame *tf, TF_TYPE type);

/**
 * Đăng ký một generic listener.
 * Register a generic listener.
 *
 * @param tf - instance
 * @param cb - callback
 * @return thành công | success
 */
bool TF_AddGenericListener(TinyFrame *tf, TF_Listener cb);

/**
 * Xóa một generic listener theo function pointer
 * Remove a generic listener by function pointer
 *
 * @param tf - instance
 * @param cb - callback function để xóa | callback function to remove
 */
bool TF_RemoveGenericListener(TinyFrame *tf, TF_Listener cb);

/**
 * Gia hạn timeout của ID listener từ bên ngoài (thay vì trả về TF_RENEW từ ID listener)
 * Renew an ID listener timeout externally (as opposed to by returning TF_RENEW from the ID listener)
 *
 * @param tf - instance
 * @param id - ID listener để gia hạn | listener ID to renew
 * @return true nếu listener được tìm thấy và gia hạn | true if listener was found and renewed
 */
bool TF_RenewIdListener(TinyFrame *tf, TF_ID id);

// ---------------------------- CÁC HÀM TRUYỀN FRAME | FRAME TX FUNCTIONS ------------------------------

/**
 * Gửi một frame, không có listener
 * Send a frame, no listener
 *
 * @param tf - instance
 * @param msg - struct message. ID được lưu trữ trong trường frame_id | message struct. ID is stored in the frame_id field
 * @return thành công | success
 */
bool TF_Send(TinyFrame *tf, TF_Msg *msg);

/**
 * Giống như TF_Send, nhưng không cần struct
 * Like TF_Send, but without the struct
 */
bool TF_SendSimple(TinyFrame *tf, TF_TYPE type, const uint8_t *data, TF_LEN len);

/**
 * Gửi một frame và tùy chọn đính kèm một ID listener.
 * Send a frame, and optionally attach an ID listener.
 *
 * @param tf - instance
 * @param msg - struct message. ID được lưu trữ trong trường frame_id | message struct. ID is stored in the frame_id field
 * @param listener - listener chờ phản hồi (có thể là NULL) | listener waiting for the response (can be NULL)
 * @param ftimeout - callback timeout
 * @param timeout - thời gian hết hạn listener tính bằng tick | listener expiry time in ticks
 * @return thành công | success
 */
bool TF_Query(TinyFrame *tf, TF_Msg *msg, TF_Listener listener,
              TF_Listener_Timeout ftimeout, TF_TICKS timeout);

/**
 * Giống như TF_Query(), nhưng không cần struct
 * Like TF_Query(), but without the struct
 */
bool TF_QuerySimple(TinyFrame *tf, TF_TYPE type,
                    const uint8_t *data, TF_LEN len,
                    TF_Listener listener, TF_Listener_Timeout ftimeout, TF_TICKS timeout);

/**
 * Gửi phản hồi cho một thông điệp đã nhận.
 * Send a response to a received message.
 *
 * @param tf - instance
 * @param msg - struct message. ID được đọc từ frame_id | message struct. ID is read from frame_id
 * @return thành công | success
 */
bool TF_Respond(TinyFrame *tf, TF_Msg *msg);

// ------------------------ CÁC HÀM TRUYỀN FRAME MULTIPART | MULTIPART FRAME TX FUNCTIONS -----------------------------
// Các routine này được sử dụng để gửi frame dài mà không cần có tất cả dữ liệu sẵn sàng
// cùng một lúc (ví dụ: capture từ peripheral hoặc đọc từ buffer bộ nhớ lớn)
// Those routines are used to send long frames without having all the data available
// at once (e.g. capturing it from a peripheral or reading from a large memory buffer)

/**
 * TF_Send() với payload multipart.
 * TF_Send() with multipart payload.
 * msg.data bị bỏ qua và đặt thành NULL
 * msg.data is ignored and set to NULL
 */
bool TF_Send_Multipart(TinyFrame *tf, TF_Msg *msg);

/**
 * TF_SendSimple() với payload multipart.
 * TF_SendSimple() with multipart payload.
 */
bool TF_SendSimple_Multipart(TinyFrame *tf, TF_TYPE type, TF_LEN len);

/**
 * TF_QuerySimple() với payload multipart.
 * TF_QuerySimple() with multipart payload.
 */
bool TF_QuerySimple_Multipart(TinyFrame *tf, TF_TYPE type, TF_LEN len, TF_Listener listener, TF_Listener_Timeout ftimeout, TF_TICKS timeout);

/**
 * TF_Query() với payload multipart.
 * TF_Query() with multipart payload.
 * msg.data bị bỏ qua và đặt thành NULL
 * msg.data is ignored and set to NULL
 */
bool TF_Query_Multipart(TinyFrame *tf, TF_Msg *msg, TF_Listener listener, TF_Listener_Timeout ftimeout, TF_TICKS timeout);

/**
 * TF_Respond() với payload multipart.
 * TF_Respond() with multipart payload.
 * msg.data bị bỏ qua và đặt thành NULL
 * msg.data is ignored and set to NULL
 */
void TF_Respond_Multipart(TinyFrame *tf, TF_Msg *msg);

/**
 * Gửi payload cho frame multipart đã bắt đầu. Có thể được gọi nhiều lần
 * nếu cần, cho đến khi độ dài đầy đủ được truyền.
 * Send the payload for a started multipart frame. This can be called multiple times
 * if needed, until the full length is transmitted.
 *
 * @param tf - instance
 * @param buff - buffer để gửi byte từ đó | buffer to send bytes from
 * @param length - số byte để gửi | number of bytes to send
 */
void TF_Multipart_Payload(TinyFrame *tf, const uint8_t *buff, uint32_t length);

/**
 * Đóng thông điệp multipart, tạo checksum và giải phóng khóa Tx.
 * Close the multipart message, generating checksum and releasing the Tx lock.
 *
 * @param tf - instance
 */
void TF_Multipart_Close(TinyFrame *tf);

// ---------------------------------- NỘI BỘ | INTERNAL ----------------------------------
// Phần này chỉ có thể nhìn thấy công khai để cho phép khởi tạo tĩnh.
// This is publicly visible only to allow static init.

// Enum trạng thái parser | Parser state enum
enum TF_State_
{
    TFState_SOF = 0,    //!< Chờ SOF | Wait for SOF
    TFState_LEN,        //!< Chờ số byte | Wait for Number Of Bytes
    TFState_HEAD_CKSUM, //!< Chờ Checksum header | Wait for header Checksum
    TFState_ID,         //!< Chờ ID | Wait for ID
    TFState_TYPE,       //!< Chờ loại thông điệp | Wait for message type
    TFState_DATA,       //!< Nhận payload | Receive payload
    TFState_DATA_CKSUM  //!< Chờ Checksum | Wait for Checksum
};

// Struct cho ID listener
// Struct for ID listener
struct TF_IdListener_
{
    TF_ID id;                       // ID frame
    TF_Listener fn;                 // Callback function
    TF_Listener_Timeout fn_timeout; // Timeout callback
    TF_TICKS timeout;               // số tick còn lại để vô hiệu hóa listener này | nr of ticks remaining to disable this listener
    TF_TICKS timeout_max;           // timeout gốc được lưu trữ ở đây (0 = không timeout) | the original timeout is stored here (0 = no timeout)
    void *userdata;                 // Dữ liệu người dùng 1 | User data 1
    void *userdata2;                // Dữ liệu người dùng 2 | User data 2
};

// Struct cho Type listener
// Struct for Type listener
struct TF_TypeListener_
{
    TF_TYPE type;   // Loại frame | Frame type
    TF_Listener fn; // Callback function
};

// Struct cho Generic listener
// Struct for Generic listener
struct TF_GenericListener_
{
    TF_Listener fn; // Callback function
};

/**
 * Trạng thái nội bộ của frame parser.
 * Frame parser internal state.
 */
struct TinyFrame_
{
    /* Dữ liệu người dùng công khai | Public user data */
    void *userdata;   // Con trỏ dữ liệu người dùng | User data pointer
    uint32_t usertag; // Tag người dùng | User tag

    // --- phần còn lại của struct là nội bộ, không truy cập trực tiếp ---
    // --- the rest of the struct is internal, do not access directly ---

    /* Trạng thái riêng | Own state */
    TF_Peer peer_bit; //!< Bit peer riêng (duy nhất để tránh xung đột ID msg) | Own peer bit (unique to avoid msg ID clash)
    TF_ID next_id;    //!< ID frame / chuỗi frame tiếp theo | Next frame / frame chain ID

    /* Trạng thái parser | Parser state */
    enum TF_State_ state;            // Trạng thái hiện tại của state machine | Current state machine state
    TF_TICKS parser_timeout_ticks;   // Tick timeout cho parser | Parser timeout ticks
    TF_ID id;                        //!< ID gói tin đến | Incoming packet ID
    TF_LEN len;                      //!< Độ dài payload | Payload length
    uint8_t data[TF_MAX_PAYLOAD_RX]; //!< Buffer byte dữ liệu | Data byte buffer
    TF_LEN rxi;                      //!< Bộ đếm byte kích thước trường | Field size byte counter
    TF_CKSUM cksum;                  //!< Checksum được tính của luồng dữ liệu | Checksum calculated of the data stream
    TF_CKSUM ref_cksum;              //!< Checksum tham chiếu đọc từ thông điệp | Reference checksum read from the message
    TF_TYPE type;                    //!< Số loại thông điệp được thu thập | Collected message type number
    bool discard_data;               //!< Đặt nếu (len > TF_MAX_PAYLOAD) để đọc frame nhưng bỏ qua dữ liệu | Set if (len > TF_MAX_PAYLOAD) to read the frame, but ignore the data.

    /* Trạng thái Tx | Tx state */
    // Buffer để xây dựng frame | Buffer for building frames
    uint8_t sendbuf[TF_SENDBUF_LEN]; //!< Buffer tạm thời truyền | Transmit temporary buffer

    uint32_t tx_pos;   //!< Vị trí ghi tiếp theo trong buffer Tx (dùng cho multipart) | Next write position in the Tx buffer (used for multipart)
    uint32_t tx_len;   //!< Tổng độ dài Tx dự kiến | Total expected Tx length
    TF_CKSUM tx_cksum; //!< Bộ tích lũy checksum truyền | Transmit checksum accumulator

#if !TF_USE_MUTEX
    bool soft_lock; //!< Cờ khóa Tx được sử dụng nếu tính năng mutex không được bật | Tx lock flag used if the mutex feature is not enabled.
#endif

    /* --- Callbacks --- */

    /* Callback giao dịch | Transaction callbacks */
    struct TF_IdListener_ id_listeners[TF_MAX_ID_LST];            // Mảng ID listeners
    struct TF_TypeListener_ type_listeners[TF_MAX_TYPE_LST];      // Mảng Type listeners
    struct TF_GenericListener_ generic_listeners[TF_MAX_GEN_LST]; // Mảng Generic listeners

    // Các bộ đếm này được sử dụng để tối ưu hóa thời gian tra cứu.
    // Chúng trỏ đến số slot được sử dụng cao nhất,
    // hoặc gần với nó, tùy thuộc vào thứ tự xóa.
    // Those counters are used to optimize look-up times.
    // They point to the highest used slot number,
    // or close to it, depending on the removal order.
    TF_COUNT count_id_lst;      // Số lượng ID listeners | Count of ID listeners
    TF_COUNT count_type_lst;    // Số lượng Type listeners | Count of Type listeners
    TF_COUNT count_generic_lst; // Số lượng Generic listeners | Count of Generic listeners
};

// ------------------------ CẦN ĐƯỢC IMPLEMENT BỞI NGƯỜI DÙNG | TO BE IMPLEMENTED BY USER ------------------------

/**
 * Hàm 'Write bytes' gửi dữ liệu đến UART
 * 'Write bytes' function that sends data to UART
 *
 * ! Implement hàm này trong mã ứng dụng của bạn !
 * ! Implement this in your application code !
 */
extern void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len);

// Các hàm Mutex | Mutex functions
#if TF_USE_MUTEX

/**
 * Chiếm giao diện TX trước khi tạo và gửi frame
 * Claim the TX interface before composing and sending a frame
 */
extern bool TF_ClaimTx(TinyFrame *tf);

/**
 * Giải phóng giao diện TX sau khi tạo và gửi frame
 * Free the TX interface after composing and sending a frame
 */
extern void TF_ReleaseTx(TinyFrame *tf);

#endif

// Các hàm checksum tùy chỉnh | Custom checksum functions
#if (TF_CKSUM_TYPE == TF_CKSUM_CUSTOM8) || (TF_CKSUM_TYPE == TF_CKSUM_CUSTOM16) || (TF_CKSUM_TYPE == TF_CKSUM_CUSTOM32)

/**
 * Khởi tạo một checksum
 * Initialize a checksum
 *
 * @return giá trị checksum ban đầu | initial checksum value
 */
extern TF_CKSUM TF_CksumStart(void);

/**
 * Cập nhật checksum với một byte
 * Update a checksum with a byte
 *
 * @param cksum - giá trị checksum trước đó | previous checksum value
 * @param byte - byte để thêm | byte to add
 * @return giá trị checksum đã cập nhật | updated checksum value
 */
extern TF_CKSUM TF_CksumAdd(TF_CKSUM cksum, uint8_t byte);

/**
 * Hoàn thành tính toán checksum
 * Finalize the checksum calculation
 *
 * @param cksum - giá trị checksum trước đó | previous checksum value
 * @return giá trị checksum cuối cùng | final checksum value
 */
extern TF_CKSUM TF_CksumEnd(TF_CKSUM cksum);

#endif

#endif
