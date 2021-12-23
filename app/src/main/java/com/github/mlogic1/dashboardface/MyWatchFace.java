package com.github.mlogic1.dashboardface;

import android.view.SurfaceHolder;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.wear.watchface.CanvasType;
import androidx.wear.watchface.ComplicationSlotsManager;
import androidx.wear.watchface.WatchFace;
import androidx.wear.watchface.WatchFaceService;
import androidx.wear.watchface.WatchFaceType;
import androidx.wear.watchface.WatchState;
import androidx.wear.watchface.style.CurrentUserStyleRepository;

import kotlin.coroutines.Continuation;

public class MyWatchFace extends WatchFaceService
{
    private DashboardFaceRenderer mRenderer;

    @Nullable
    @Override
    protected Object createWatchFace(@NonNull SurfaceHolder surfaceHolder, @NonNull WatchState watchState, @NonNull ComplicationSlotsManager complicationSlotsManager, @NonNull CurrentUserStyleRepository currentUserStyleRepository, @NonNull Continuation<? super WatchFace> continuation) {
        mRenderer = new DashboardFaceRenderer(getApplicationContext(), surfaceHolder, watchState, complicationSlotsManager, currentUserStyleRepository, CanvasType.HARDWARE);

        return new WatchFace(WatchFaceType.ANALOG, mRenderer);
    }
}