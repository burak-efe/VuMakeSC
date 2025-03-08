#pragma once

#include "VuBuffer.h"


namespace Vu {
    struct VuBindlessConfigInfo;

    template<typename T>
    struct VuHandle;

    struct ResourceCounters {
        uint32 referanceCounter;
        uint32 generationCounter;
    };

    //TObj should implement uninit()
    //this class is not incrementing or decrementing refCount itselfs, object that receives(or sends it) should do that instead
    template<typename TObj>
    struct VuPool {
        inline static std::vector<TObj>   data;
        inline static std::vector<ResourceCounters> counters;
        inline static std::stack<uint32>  freeList;

        static TObj* get(uint32 index, uint32 generation) {
            if (generation != counters[index].generationCounter) {
                return nullptr;
            }
            return &data[index];
        }

        static uint32 getUsedSlotCount() {
            return data.size() - freeList.size();
        }

        //allocate, set refCount to one, and return index
        static void allocate(uint32& index, uint32& generation) {
            if (!freeList.empty()) {
                uint32 i = freeList.top();
                freeList.pop();
                counters[i].referanceCounter = 1;
                index        = i;
                generation   = counters[index].generationCounter;
                return;
            }
            data.push_back(TObj{});
            counters.push_back({1,0});
            index      = data.size() - 1;
            generation = counters[index].generationCounter;
        }

        static void increaseRefCount(uint32 index) {
            counters[index].referanceCounter += 1;
        }

        static VkBool32 decreaseRefCount(uint32 index) {
            counters[index].referanceCounter -= 1;

            if (counters[index].referanceCounter == 0) {
                //delete
                data[index].uninit();
                freeList.push(index);
                counters[index].generationCounter++;
                return true;
            }

            if (counters[index].referanceCounter < 0) {
                std::cerr << "Referance count of object below zero" << std::endl;
            }
            return false;
        }
    };

    template<typename T>
    struct VuHandle {
        uint32 index;
        uint32 generation;

        //alloc a slot from pool and return the unitialized object
        T* createHandle() {
            VuPool<T>::allocate(index, generation);
            return VuPool<T>::get(index, generation);
        }
        //return true if reference count drops == 0, which meand you need to uninit the object
        VkBool32 destroyHandle() {
            return VuPool<T>::decreaseRefCount(index);
        }

        T* get() {
            return VuPool<T>::get(index, generation);
        }
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct VuResourceManager {
    private:
        inline static VuBuffer bufferOfStorageBuffer;

    public:
        static void init(const VuBindlessConfigInfo& info);

        static void uninit();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        static void registerStorageBuffer(uint32 writeIndex, const VuBuffer& buffer);

        static void writeStorageBuffer(const VuBuffer& buffer, uint32 binding);

        static void writeSampledImageToGlobalPool(uint32 writeIndex, const VkImageView& imageView);

        static void writeSamplerToGlobalPool(uint32 writeIndex, const VkSampler& sampler);

        static void writeUBO_ToGlobalPool(uint32 writeIndex, uint32 setIndex, const VuBuffer& buffer);
    };
}
