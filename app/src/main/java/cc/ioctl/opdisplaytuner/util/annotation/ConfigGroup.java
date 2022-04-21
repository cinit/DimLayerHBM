package cc.ioctl.opdisplaytuner.util.annotation;

import androidx.annotation.NonNull;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Target({ElementType.TYPE, ElementType.FIELD})
@Retention(RetentionPolicy.RUNTIME)
public @interface ConfigGroup {
    // TODO: 2021-12-21 add KAPT processing for this annotation
    @NonNull String value();
}
