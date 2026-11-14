package com.example.masquerade;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
                "SDL3",
                "masquerade"
        };
    }
}