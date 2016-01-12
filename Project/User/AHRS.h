// 变量链接
extern float Pitch, Roll, Yaw;
extern float heading;

// 函数定义
void Quat_Init(void);
void AHRS_Update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
