

var SND = {
	/*
  SNDDMA_Init: SNDDMA_Init,
  SNDDMA_Shutdown: SNDDMA_Shutdown,
  SNDDMA_BeginPainting: SNDDMA_BeginPainting,
  SNDDMA_Submit: SNDDMA_Submit,
  SNDDMA_GetDMAPos: SNDDMA_GetDMAPos,
	*/
	SDL_GetTicks: Sys_Milliseconds,
	SDL_Delay: SDL_Delay,

}

function SDL_Delay(delay) {
	// horrible busy-wait, but in a worker it at least does not block rendering
	//var now = Date.now()
	//while (Date.now() - now < delay) {}
}

