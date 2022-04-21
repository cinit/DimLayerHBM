package cc.ioctl.opdisplaytuner.startup;

import cc.ioctl.opdisplaytuner.startup.step.EarlyInit;
import cc.ioctl.opdisplaytuner.startup.step.EmptyStep;

public class StepFactory {

    public static final int STEP_EARLY_INIT = 10;

    public static Step getStep(int id) {
        switch (id) {
            case STEP_EARLY_INIT:
                return new EarlyInit();
            default:
                return new EmptyStep();
        }
    }
}
