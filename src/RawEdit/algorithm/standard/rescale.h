#pragma once

#include "../base/algorithm.h"

namespace RawEdit
{
    template<typename T>
    Error NearestCPU(const CPUImage<T>* input, CPUImage<T>* output, uint32_t width, uint32_t height)
    {
        output->Resize(width, height, input->channels);
        const float wratio = input->width  / (float)width;
        const float hratio = input->height / (float)height;

        #pragma omp parallel for collapse(2)
        for (uint32_t i = 0; i < height; ++i)
        {
            for (uint32_t j = 0; j < width; ++j)
            {
                const uint32_t srcX = std::min((uint32_t)(j * wratio), input->width - 1);
                const uint32_t srcY = std::min((uint32_t)(i * hratio), input->height - 1);

                for (uint32_t k = 0; k < input->channels; ++k)
                    output->SetData(i, j, k, input->GetData(srcY, srcX, k));
            }
        }
        
        return Ok();
    }

    class Rescale : public Algorithm
    {
    public:
        Rescale() : Algorithm("Rescale")
        {
            inputs["method"] = EnumType({"Nearest", "Bilinear", "Bicubic"}, 0);
            inputs["factor"] = 1.f;

            for (auto& it : inputs)
                it.second.multiMask = false;
        }

        Error Run() override
        {
            Error err;
            DISPATCH_IMAGE_CALL(inputImage, {
                auto in = inputImage.get();
                auto out = outputImage.get();

                err = Run(reinterpret_cast<ImagePtr>(in), reinterpret_cast<ImagePtr>(out));
            });
            return err;
        }

    private:
        template<typename T>
        Error Run(CPUImage<T>* input, CPUImage<T>* output)
        {
            const float factor = inputs["factor"].AsFloat();
            const uint32_t tWidth  = input->width  * factor;
            const uint32_t tHeight = input->height * factor;
            
            const std::string& method = inputs["method"].AsEnum().value;

            if (method == "Nearest")
                return NearestCPU(input, output, tWidth, tHeight);

            return Error("Unknown set of parameters for rescale with CPUImage");
        }

        template<typename T>
        Error Run(T i, T o)
        {
            std::cout << "Not implemented " << std::endl;
            return Error("Run method not implemented for Resc");
        }
    };
};
