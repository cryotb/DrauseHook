//
// Created by drof on 26.01.24.
//

#ifndef BACKEND_SOCKETS_WRAPPER_H
#define BACKEND_SOCKETS_WRAPPER_H

namespace sock
{
    inline int recursive_send(int socket, const void* pdata, size_t size)
    {
        auto data = (const char*)pdata;
        if (size == 0) {
            return 0;  // Nothing to send
        }

        ssize_t sent_bytes = send(socket, data, size, 0);

        if (sent_bytes == -1) {
            // Handle error, e.g., print an error message or return an error code
            return -1;
        }

        if (sent_bytes < static_cast<ssize_t>(size)) {
            // Call send recursively for the remaining data
            return sent_bytes + recursive_send(socket, data + sent_bytes, size - sent_bytes);
        }

        return sent_bytes;
    }

    inline int recursive_recv(int socket, void* pbuffer, size_t size)
    {
        auto buffer= (char*)pbuffer;
        if (size == 0) {
            return 0;  // Nothing to receive
        }

        ssize_t received_bytes = recv(socket, buffer, size, 0);

        if (received_bytes == -1) {
            // Handle error, e.g., print an error message or return an error code
            return -1;
        }

        if (received_bytes == 0) {
            // Connection closed by the other side
            return 0;
        }

        // Call recv recursively for the remaining data
        return received_bytes + recursive_recv(socket, buffer + received_bytes, size - received_bytes);
    }

    inline int recursive_recv_buffered(int socket, void* pbuffer, int size, int chunksize, int chunkratio)
    {
        int bytes_received = 0;
        int bytes_left = size;

        auto data = reinterpret_cast<u8*>(pbuffer);

        auto bytes_per_chunk = chunksize / chunkratio;

        double last_percent = 0.f;

        while (bytes_left > 0)
        {
            if(bytes_left < bytes_per_chunk)
                bytes_per_chunk = bytes_left;

            int result = recv(socket, &data[bytes_received], bytes_per_chunk, 0);
            if (result > 0)
            {
                double current_percent = (static_cast<double>(bytes_received) / size) * 100;
                if( (int)last_percent != (int)current_percent)
                {
                    printf("[DL][BUFFERING] %.0f%%\n",
                       current_percent);

                    last_percent = current_percent;
                }
                bytes_left -= result;
                bytes_received += result;
            } else
            {
                if(result < 0)
                {
                    return -1;
                }
            }
        }

        return bytes_received;
    }
}

#endif //BACKEND_SOCKETS_WRAPPER_H

