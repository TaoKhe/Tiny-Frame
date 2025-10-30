#include "TinyFrame.h"

/**
 * Đây là một ví dụ về việc tích hợp TinyFrame vào ứng dụng.
 * This is an example of integrating TinyFrame into the application.
 *
 * TF_WriteImpl() là bắt buộc, các hàm mutex là weak và có thể
 * được loại bỏ nếu không sử dụng. Chúng được gọi từ tất cả hàm TF_Send/Respond.
 * TF_WriteImpl() is required, the mutex functions are weak and can
 * be removed if not used. They are called from all TF_Send/Respond functions.
 *
 * Cũng nhớ gọi TF_Tick() định kỳ nếu bạn muốn sử dụng
 * tính năng timeout của listener.
 * Also remember to periodically call TF_Tick() if you wish to use the
 * listener timeout feature.
 */

void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    // gửi đến UART | send to UART
}

// --------- Callback Mutex | Mutex callbacks ----------
// Chỉ cần nếu TF_USE_MUTEX là 1 trong file config.
// XÓA nếu mutex không được sử dụng
// Needed only if TF_USE_MUTEX is 1 in the config file.
// DELETE if mutex is not used

/** Chiếm giao diện TX trước khi tạo và gửi frame | Claim the TX interface before composing and sending a frame */
bool TF_ClaimTx(TinyFrame *tf)
{
    // lấy mutex | take mutex
    return true; // chúng ta thành công | we succeeded
}

/** Giải phóng giao diện TX sau khi tạo và gửi frame | Free the TX interface after composing and sending a frame */
void TF_ReleaseTx(TinyFrame *tf)
{
    // giải phóng mutex | release mutex
}

// --------- Checksum tùy chỉnh | Custom checksums ---------
// Điều này chỉ nên được định nghĩa ở đây nếu loại checksum tùy chỉnh được sử dụng.
// XÓA những thứ này nếu bạn sử dụng một trong các loại checksum có sẵn
// This should be defined here only if a custom checksum type is used.
// DELETE those if you use one of the built-in checksum types

/** Khởi tạo một checksum | Initialize a checksum */
TF_CKSUM TF_CksumStart(void)
{
    return 0;
}

/** Cập nhật checksum với một byte | Update a checksum with a byte */
TF_CKSUM TF_CksumAdd(TF_CKSUM cksum, uint8_t byte)
{
    return cksum ^ byte;
}

/** Hoàn thành tính toán checksum | Finalize the checksum calculation */
TF_CKSUM TF_CksumEnd(TF_CKSUM cksum)
{
    return cksum;
}
