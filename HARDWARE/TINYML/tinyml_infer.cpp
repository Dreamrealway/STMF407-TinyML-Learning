#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/schema/schema_generated.h"
#include "model_data.h"

extern "C" {
#include "lcd.h"
}

constexpr int kTensorArenaSize = 48 * 1024;
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));

static tflite::MicroMutableOpResolver<10> resolver;  // 10个算子
static tflite::MicroInterpreter *interpreter = nullptr;
static bool tinyml_ready = false;

static int tinyml_init(void)
{
    const tflite::Model* model = tflite::GetModel(mnist_model_data);
    LCD_ShowNum(30, 250, model->version(), 1, 16);  // 显示版本号
    if (model->version() != TFLITE_SCHEMA_VERSION){
        LCD_ShowString(30, 210, 240, 16, 16, (u8*)"model version mismatch");
        return -1;
    }

    resolver.AddConv2D();
    resolver.AddMaxPool2D();
    resolver.AddFullyConnected();
    resolver.AddReshape();
    resolver.AddQuantize();
    resolver.AddDequantize();
    resolver.AddMul();      // ← 加这个，Rescaling 可能转成 MUL
    resolver.AddShape();        // ← 加这个
    resolver.AddStridedSlice(); // ← 加这个
    resolver.AddPack();         // ← 加这个

    static tflite::MicroInterpreter interp(
        model, resolver, tensor_arena, kTensorArenaSize);
    interpreter = &interp;

    if (interpreter->AllocateTensors() != kTfLiteOk){
        LCD_ShowString(30, 210, 240, 16, 16, (u8*)"AllocateTensors failed");
        return -1;
    }

    LCD_ShowString(30, 210, 240, 16, 16, (u8*)"init ok");
    return 0;
}

extern "C" int tinyml_predict(const int8_t *input, int8_t *output)
{
    if (!tinyml_ready) {
        if (tinyml_init() != 0)
            return -1;
        tinyml_ready = true;
    }

    TfLiteTensor* input_tensor = interpreter->input(0);
    for (int i = 0; i < 28 * 28; i++)
        input_tensor->data.int8[i] = input[i];

    if (interpreter->Invoke() != kTfLiteOk){
        LCD_ShowString(30, 230, 240, 16, 16, (u8*)"Invoke failed");
        return -1;
    }

    TfLiteTensor* output_tensor = interpreter->output(0);
    for (int i = 0; i < 10; i++)
        output[i] = output_tensor->data.int8[i];

    return 0;
}