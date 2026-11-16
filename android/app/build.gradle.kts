plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.example.masquerade"
    compileSdk = 37

    sourceSets {
        getByName("main") {
            java.directories.add("src/main/java")
            java.directories.add("C:/Data/Softwares/SDL3/SDL/android-project/app/src/main/java")
            assets.directories.add("../../assets")
        }
    }

    defaultConfig {
        applicationId = "com.example.masquerade"
        minSdk = 28
        targetSdk = 37
        versionCode = 7014
        versionName = "0.7014"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        ndk {
            abiFilters += listOf("arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                // Keep shared flags and arguments here
                cppFlags += "-std=c++23"

                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DSDL_SHARED=ON",
                    "-DIMGUI_IMPL_OPENGL_ES3=ON"
                )
            }
        }
    }

    buildTypes {
        getByName("debug") {
            isJniDebuggable = true
            externalNativeBuild {
                cmake {
                    // Force no optimization for easy debugging
                    cppFlags += "-O0"
                }
            }
        }

        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
            signingConfig = signingConfigs.getByName("debug")

            externalNativeBuild {
                cmake {
                    // Force -O3 optimization for release builds
                    cppFlags += "-O3"
                }
            }
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            version = "3.22.1+"
        }
    }
}

dependencies {
    implementation(libs.androidx.appcompat)
    implementation(libs.androidx.core.ktx)
    implementation(libs.material)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(libs.androidx.junit)
}