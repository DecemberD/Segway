class Motor
{
public:
    int StepPresc = 0;				// Revolution control prescaler
    int MicStep = 0;			// Sine-Cosine look-up array index (MicStepArray[])
    static const int MICSTEPS = 16;
    static int* MotorCurve;
    int StepIncLatch = 1;
    int StepInc = 0;
    int StepPrescVal = 0;
    int StepPrescValLatch = 1250;
    int SpeedAbs = 0;
    int Speed = 0;
    int Direction = 1;
    static const int SPEED_SLEWRATE = 1;
public:
    int SpeedSlewRate(const int SR, int Speed);


};

int Motor::SpeedSlewRate(const int SR, int Speed)
{
    static int CurrSpeed = 0;
    if(Speed > CurrSpeed)
    {
        CurrSpeed += SR;
        if(CurrSpeed > Speed)
            CurrSpeed = Speed;
    }
//    else(Speed < CurrSpeed)
//    {
//        CurrSpeed -= SR;
//        if(CurrSpeed < Speed)
//            CurrSpeed = Speed;
//    }
    return CurrSpeed;
}

int main()
{
    int i = 0;
    i++;
    class Motor motor;
}