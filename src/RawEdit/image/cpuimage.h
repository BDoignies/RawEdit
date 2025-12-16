#pragma once

#include "imagebase.h"
#include <cstring>

namespace RawEdit
{
    template<typename T>
    class CPUImage : public ImageBase
    {
    public:
        CPUImage() : ImageBase(ImageBackend::CPU, TypeToImageDataType<T>())
        { }

        ImageBase* EmptyCopy(bool mtdata) const override
        {
            CPUImage<T>* newImg = new CPUImage<T>();
            if (mtdata) 
                newImg->metadata = metadata;
            return newImg;
        }

        ImageBase* Copy() const override
        {
            CPUImage<T>* newImage = new CPUImage<T>();
            newImage->metadata = metadata;
            newImage->SetData(width, height, channels, type, data);
            return newImage;
        }

        void Resize(uint32_t w, uint32_t h, uint32_t c = 0)
        {
            if (c == 0) c = channels;
            width = w;
            height = h;
            channels = c;

            delete[] data;
            data = new T[w * h * c];
        }

        uint32_t GetIndex(uint32_t i, uint32_t j, uint32_t c = 0) const
        {
            return c + (j + i * width) * channels;
        }

        template<typename U>
        void SetData(uint32_t i, U val) { data[i] = val; }
        template<typename U>
        void SetData(uint32_t i, uint32_t j, U val) { data[GetIndex(i, j, 0)] = val; }
        template<typename U>
        void SetData(uint32_t i, uint32_t j, uint32_t c, U val) { data[GetIndex(i, j, c)] = val; }
        
        T& GetData(uint32_t i)       { return data[i]; }
        T  GetData(uint32_t i) const { return data[i]; }
        T& GetData(uint32_t i, uint32_t j, uint32_t c = 0)       { return data[GetIndex(i, j, c)]; }
        T  GetData(uint32_t i, uint32_t j, uint32_t c = 0) const { return data[GetIndex(i, j, c)]; }

        T* GetDataPtr() { return data; }
        const T* GetDataPtr() const { return data; }
        
        template<typename U>
        void FillData(uint32_t w, uint32_t h, uint32_t c, const U& val)
        {
            if (data == nullptr || w != width || h != height || c != channels)
            {
                delete[] data;
                data = new T[w * h * c];
            }

            for (uint32_t i = 0; i < w * h * c; ++i)
                data[i] = val;

            width = w;
            height = h;
            channels = c;
        }

        virtual void SetData(
            uint32_t w, uint32_t h, uint32_t c, 
            ImageDataType newdatatype, const void* newdata
        )
        {
            if (w != width || h != height || c != channels)
            {
                delete[] data;
                data = new T[w * h * c];
            }

            width = w;
            height = h;
            channels = c;
            
            if (type == newdatatype)
            {
                memcpy(data, newdata, w * h * c * SizeofType(type));
            }
            else
            {
                DISPATCH_DATATYPE(newdatatype,
                    const DataType* typedData = reinterpret_cast<const DataType*>(newdata);
                    for (size_t i = 0; i < w * h * c; ++i)
                        data[i] = typedData[i];
                );
            }
        }

        ~CPUImage()
        {
            delete[] data;
        }
    protected:
        T* data = nullptr;
    };
}
