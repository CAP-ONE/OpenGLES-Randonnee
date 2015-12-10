/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.androidGL4D;

import android.app.Activity;
import android.os.Bundle;

import com.google.vrtoolkit.cardboard.CardboardActivity;
import com.google.vrtoolkit.cardboard.CardboardView;


public class AGL4DActivity extends CardboardActivity {

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.common_ui);

        AGL4DView cardboardView = (AGL4DView) findViewById(R.id.cardboard_view);

        setCardboardView(cardboardView);

        CardboardOverlayView overlayView = (CardboardOverlayView) findViewById(R.id.overlay);
    }
}
