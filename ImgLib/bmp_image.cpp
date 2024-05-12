#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>
#include <cstdint>

using namespace std;

namespace img_lib {

    PACKED_STRUCT_BEGIN BitmapFileHeader{
        // поля заголовка Bitmap File Header
        char signature_b;
        char sigmature_m;
        unsigned int sum_size;
        unsigned int reserved;
        unsigned int header_size;
    }
    PACKED_STRUCT_END

    PACKED_STRUCT_BEGIN BitmapInfoHeader{
        // поля заголовка Bitmap Info Header
        unsigned int info_header_size;
        int width;
        int height;
        uint16_t number_planes;
        uint16_t number_bits_per_pixel;
        unsigned int compres_type;
        unsigned int number_bytes_in_data;
        int gorizont_def;
        int vertical_def;
        int number_colors_used;
        int number_colors_;
    }
        PACKED_STRUCT_END

    const int number_colors = 3;
    const int align = 4;


// функция вычисления отступа по ширине
    static int GetBMPStride(int w) {
        return align * ((w * number_colors + number_colors) / align);
    }

// напишите эту функцию
    bool SaveBMP(const Path& file, const Image& image) {
        ofstream out(file, ios::binary);
        unsigned int sum_size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + static_cast<unsigned int>(GetBMPStride(image.GetWidth()) * image.GetHeight());
        unsigned int number_bytes_in_data = GetBMPStride(image.GetWidth()) * image.GetHeight();
        uint16_t number_planes = 1;
        uint16_t number_bits_per_pixel = 24;
        unsigned int compres_type = 0;
        int gorizont_def = 11811;
        int vertical_def = 11811;
        int number_colors_used = 0;
        int number_colors_ = 0x1000000;

        BitmapFileHeader bfh{ 'B' , 'M', sum_size, 0, sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) };
        BitmapInfoHeader bih{ sizeof(BitmapInfoHeader), image.GetWidth(),  image.GetHeight() , number_planes, number_bits_per_pixel, compres_type, number_bytes_in_data, gorizont_def, vertical_def, number_colors_used, number_colors_ };

        out.write(reinterpret_cast<const char*>(&bfh), sizeof(bfh));
        out.write(reinterpret_cast<const char*>(&bih), sizeof(bih));
        
        const int w = image.GetWidth();
        const int h = image.GetHeight();
        int width_size = GetBMPStride(w);
        std::vector<char> buff(width_size);

        for (int y = h - 1; y >= 0; --y) {
            const Color* line = image.GetLine(y);
            for (int x = 0; x < w; ++x) {
                buff[x * 3 + 2] = static_cast<char>(line[x].r);
                buff[x * 3 + 1] = static_cast<char>(line[x].g);
                buff[x * 3 + 0] = static_cast<char>(line[x].b);
            }
            out.write(buff.data(), width_size);
        }
        return out.good();
}

// напишите эту функцию
    Image LoadBMP(const Path& file) {
        ifstream ifs(file, ios::binary);
        BitmapFileHeader bfh;
        BitmapInfoHeader bih;
        if (!ifs.read(reinterpret_cast<char*>(&bfh), sizeof(bfh))) {
            return Image{};
        };
        
        if (!ifs.read(reinterpret_cast<char*>(&bih), sizeof(bih))) {
            return Image{};
        };
        const int w = bih.width;
        const int h = bih.height;
        unsigned int sum_size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + GetBMPStride(w) * h;
        if (bfh.signature_b != 'B' || bfh.sigmature_m != 'M' || bfh.sum_size != sum_size || bfh.reserved != 0 || bfh.header_size != sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader)) {
            return Image{};
        }
        
        unsigned int number_bytes_in_data = GetBMPStride(w) * h;
        uint16_t number_planes = 1;
        uint16_t number_bits_per_pixel = 24;
        unsigned int compres_type = 0;
        int gorizont_def = 11811;
        int vertical_def = 11811;
        int number_colors_used = 0;
        int number_colors_ = 0x1000000;

        if (bih.number_bytes_in_data != number_bytes_in_data || bih.number_planes != number_planes || bih.number_bits_per_pixel != number_bits_per_pixel ||
            bih.compres_type != compres_type || bih.gorizont_def != gorizont_def || bih.vertical_def != vertical_def || bih.number_colors_used != number_colors_used || bih.number_colors_ != number_colors_) {
            return Image{};
        }

        
        int width_size = GetBMPStride(w);
        Image result(w, h, Color::Black());
        std::vector<char> buff(width_size);

        for (int y = h - 1; y >=0 ; --y) {
            Color* line = result.GetLine(y);
            ifs.read(buff.data(), width_size);

            for (int x = 0; x < w; ++x) {
                line[x].r = static_cast<byte>(buff[x * 3 + 2]);
                line[x].g = static_cast<byte>(buff[x * 3 + 1]);
                line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            }
        }
        return result;
    }
}  // namespace img_lib