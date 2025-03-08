
struct Span<T> {
    Ptr<T> _pointer;
    uint64_t _length;

    __init(uint64_t ptr, uint64_t lenght)
    {
        _pointer = (Ptr<T>)ptr;
        _length = lenght;
    }

    __subscript(uint64_t index) -> T
    {
        get { return _pointer[index]; }
        set { _pointer[index] = newValue; }
    }

    // //Be careful this will not change the size of the span, its just casting type to another
    // Span<T2> CastType<T2>()
    // {
    //     Span<T2> casted;
    //     casted._pointer = (Ptr<T2>)this._pointer;
    //     casted._length = this._length;
    //     return casted;
    // }

    // Span<T3> Reinterpret<T3>()
    // {
    //     Span<T3> rep;
    //     rep._pointer = (Ptr<T3>)this._pointer;
    //     uint64_t byteSize  = sizeof(T) *  _length;
    //     rep._length = byteSize / sizeof(T3);
    //     return rep;
    // }

    //Create a new subspan
    Span<T> Slice(uint64_t _pointerIndex, uint64_t _length){
        Span<T> sliced;
        sliced._pointer = &this._pointer[_pointerIndex];
        sliced._length = _length;
        return sliced;
    }
}

[Flags]
enum MeshFlags : uint32_t {
    HasNormal,
    HasTangent,
    HasUV_0,
    HasUV_1,
    HasUV_2,
}

struct Mesh {
    uint32_t vertexBufferIndex;
    uint32_t vertexCount;
    MeshFlags flags;

    Ptr<float3>  getPositionPtr()
    {
        return (Ptr<float3>) globalStorageBuffers[vertexBufferIndex];
    }

    Ptr<float3>  getNormalPtr()
    {
        uint64_t prev = (uint64_t)getPositionPtr();
        uint64_t p = prev + sizeof(float3) * vertexCount;
        return (Ptr<float3>) p;
    }

    Ptr<float4> getTangentPtr()
    {
        uint64_t prev = (uint64_t)getNormalPtr();
        uint64_t p = prev + sizeof(float3) * vertexCount;
        return (Ptr<float4>) p;
    }

    Ptr<float2> getUV_Ptr()
    {
        uint64_t prev = (uint64_t)getTangentPtr();
        uint64_t p = prev + sizeof(float4) * vertexCount;
        return (Ptr<float2>) p;
    }


    // Span<float3> getPositions()
    // {
    //     return Span<float3>(globalStorageBuffers[vertexBufferIndex], vertexCount);
    // }

    // Span<float3> getNormals()
    // {
    //     var offset = sizeof(float3) * vertexCount;
    //     return Span<float3>(globalStorageBuffers[vertexBufferIndex] + offset, vertexCount);
    // }

    // Span<float3> getTangents()
    // {
    //     var offset = sizeof(float3) * vertexCount;
    //     return Span<float3>(globalStorageBuffers[vertexBufferIndex] + offset + offset, vertexCount);
    // }
}

struct FrameConst
{
    float4x4 view;
    float4x4 proj;
    float4 _cameraPos;
    float4 _cameraDir;
    float _time;
    float _debugIndex;

    property float3 cameraPos{ get {return  _cameraPos.xyz; }};
    property float3 cameraDir{ get {return  _cameraDir.xyz; }};
    property float time{ get {return  _time.x; }};

};

struct PushConsts
{
    float4x4 model;
    uint32_t materialBlockIndex;
    Mesh mesh;
};

[[vk::push_constant]]
PushConsts pc;

struct PBRMaterialData {
    uint32_t baseColorTexture;
    uint32_t normalTexture;
    float3 baseColorMul;
    uint32_t padding[11];
};

[[vk::binding(0, 0)]]
ConstantBuffer<FrameConst> frameConst;

//todo engine constants

[[vk::binding(1, 0)]]
SamplerState globalSamplers[];

[[vk::binding(2, 0)]]
Texture2D globalSampledImages[];

[[vk::binding(3, 0)]]
Texture2D globalStorageImages[];

[[vk::binding(4, 0)]]
StructuredBuffer<uint64_t> globalStorageBuffers;
//buffer at index 0 is globalmaterialDataBuffer

PBRMaterialData getMaterialData(uint32_t index)
{
    static_assert(sizeof(PBRMaterialData) == 64, "PBRMaterialData size is not 64 bytes");
    var globalMaterialDataBuffer = (PBRMaterialData*)globalStorageBuffers[0];
    return globalMaterialDataBuffer[index];
}


struct VSOutput
{
    float4 Pos : SV_POSITION;
    [[vk::location(0)]]float3 Normal : NORMAL0;
    [[vk::location(1)]]float3 Tangent : TANGENT0;
    [[vk::location(2)]]float3 Bitangent : BINORMAL0;
    [[vk::location(3)]]float2 UV : TEXCOORD0;
    [[vk::location(4)]]float3 PosWS : TEXCOORD1;

};
