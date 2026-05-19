# Keep Tenjin SDK
-keep class com.tenjin.** { *; }
-keep public class com.google.android.gms.ads.identifier.** { *; }
-keep public class com.google.android.gms.appset.** { *; }
-keep public class com.google.android.gms.common.** { *; }
-keep public class com.android.installreferrer.** { *; }
-keep class * extends java.util.ListResourceBundle {
    protected Object[][] getContents();
}
-keepattributes *Annotation*
