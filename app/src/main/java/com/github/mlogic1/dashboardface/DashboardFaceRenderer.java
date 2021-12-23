package com.github.mlogic1.dashboardface;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.Log;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.wear.watchface.ComplicationSlotsManager;
import androidx.wear.watchface.Renderer.CanvasRenderer2;
import androidx.wear.watchface.WatchState;
import androidx.wear.watchface.style.CurrentUserStyleRepository;
import androidx.wear.watchface.style.UserStyle;

import com.github.mlogic1.dashboardface.Interface.IFaceRenderer;

import java.time.ZonedDateTime;

import kotlin.coroutines.Continuation;

public class DashboardFaceRenderer extends CanvasRenderer2<DashboardFaceRenderer.AnalogSharedAssets> implements IFaceRenderer
{
    // private Context mContext;
    private WatchCallbacks eventsWrapper;
    private Bitmap mBitmapGauge;
    private Paint mBackgroundPaint;

    public DashboardFaceRenderer(Context context, SurfaceHolder surfaceHolder, WatchState watchState, ComplicationSlotsManager complicationSlotsManager, CurrentUserStyleRepository currentUserStyleRepository, int canvasType)
    {
        super(surfaceHolder, currentUserStyleRepository, watchState, canvasType, 16L, false);
        eventsWrapper = new WatchCallbacks(currentUserStyleRepository, this);
        mBitmapGauge = BitmapFactory.decodeResource(context.getResources(), R.drawable.gauge);
        mBackgroundPaint = new Paint();
        mBackgroundPaint.setColor(Color.GRAY);
    }

    @Override
    public void UpdateWatchFaceData(UserStyle userStyle)
    {
        Log.d("Filip", "User style changed");
    }

    /*public void UpdateWatchFaceData(UserStyle userStyle)
    {

    }*/

    class AnalogSharedAssets implements SharedAssets {

        @Override
        public void onDestroy() {

        }
    }

    @Nullable
    @Override
    protected Object createSharedAssets(@NonNull Continuation<? super AnalogSharedAssets> continuation) {
        return new AnalogSharedAssets();
    }

    @Override
    public void render(@NonNull Canvas canvas, @NonNull Rect rect, @NonNull ZonedDateTime zonedDateTime, @NonNull AnalogSharedAssets analogSharedAssets) {
        canvas.drawBitmap(mBitmapGauge, 100.0f, 100.0f, mBackgroundPaint);
    }

    @Override
    public void renderHighlightLayer(@NonNull Canvas canvas, @NonNull Rect rect, @NonNull ZonedDateTime zonedDateTime, @NonNull AnalogSharedAssets analogSharedAssets) {

    }

}
