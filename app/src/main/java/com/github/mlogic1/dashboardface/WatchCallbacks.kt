package com.github.mlogic1.dashboardface

import androidx.wear.watchface.style.CurrentUserStyleRepository
import com.github.mlogic1.dashboardface.Interface.IFaceRenderer
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch

class WatchCallbacks(
    currentUserStyleRepository: CurrentUserStyleRepository,
    dashRenderer: IFaceRenderer
){
    private val scope: CoroutineScope =
        CoroutineScope(SupervisorJob() + Dispatchers.Main.immediate)

    init {

        scope.launch{
            currentUserStyleRepository.userStyle.collect{
                    userStyle -> dashRenderer.UpdateWatchFaceData(userStyle)
            }
        }
    }
}