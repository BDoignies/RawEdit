#pragma once

#include <iostream>
#include <map>
#include "utils/error.h"
#include "utils/cache.h"
#include "params.h"

namespace RawEdit
{
    struct Algorithm
    {
    public:
        Algorithm(const std::string& n) : name(n) {}
        
        Param& operator[](const std::string& name) {
            static Param unknown;

            auto iIt = inputs.find(name);
            if (iIt != inputs.end()) return iIt->second;

            auto oIt = outputs.find(name);
            if (oIt != outputs.end()) return oIt->second;
            
            return unknown;
        };

        const Param& operator[](const std::string& name) const {
            static Param unknown;

            auto iIt = inputs.find(name);
            if (iIt != inputs.end()) return iIt->second;

            auto oIt = outputs.find(name);
            if (oIt != outputs.end()) return oIt->second;
            
            return unknown;
        };
        std::map<std::string, Param>& GetInputs()  { return inputs; }
        std::map<std::string, Param>& GetOutputs() { return outputs; }
        
        // This has sense, only non masked parameter can be bound
        virtual Error Bind(const std::string& pname, const Param* ptr)
        {
            auto it = inputs.find(name);
            if (it == inputs.end())
                return Failed("No parameter named '{}' in '{}'", pname, name).error();

            if (it->second.type != ptr->type)
                return Failed("Type mismatch for parameter '{}' in '{}'. Expected '{}', given '{}'", pname, name, ParamTypeToString(it->second.type), ParamTypeToString(ptr->type)).error();

            if (it->second.multiMask != ptr->multiMask)
                return Failed("Mask mismatch for parameter '{}' in '{}'.", pname, name).error();

            connections[pname] = ptr;
            return Ok();
        }

        virtual void BindInputImage (ImagePtr img) {  inputImage = img; }
        virtual void BindOutputImage(ImagePtr img) { outputImage = img; }
        virtual void BindMask(ImagePtr img) { mask = img; }

        void Propagate()
        {
            for (const auto& [name, ptr] : connections)
            {
                // We can safely copy here, has both have the same type
                inputs[name].values = ptr->values;
            }
        }

        void Print()
        {
            std::cout << "Algorithm: " << name << "\n";
            std::cout << "Inputs: \n";
            for (auto it : inputs)
            {
                std::cout << "\t" << it.first << " [" << ParamTypeToString(it.second.type) << "]\n";
            }
        }

        virtual Error Run() = 0;
        virtual ~Algorithm() {}
    protected:
        const std::string name;

        ImagePtr inputImage  = nullptr;
        ImagePtr outputImage = nullptr;
        ImagePtr mask        = nullptr;

        std::map<std::string, Param> inputs;
        std::map<std::string, Param> outputs;
        std::map<std::string, const Param*> connections;
    };
};
