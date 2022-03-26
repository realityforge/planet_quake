
var SND = {
  channels: 2,
  nextPlayTime: 0,
  bufferingDelay: 50 / 1000,
  SNDDMA_Init: SNDDMA_Init,
  SNDDMA_Shutdown: SNDDMA_Shutdown,
  SNDDMA_BeginPainting: SNDDMA_BeginPainting,
  SNDDMA_Submit: SNDDMA_Submit,
  SNDDMA_GetDMAPos: SNDDMA_GetDMAPos,

}

const smoothingInterval = 0.02
const beepLengthInSeconds = 0.5
const beeps = [220,440,880]

let buffer_pos = 0

function SNDDMA_AudioCallback_Test(parameter, osc) {
	let pos = Math.floor(SND.audioContext.currentTime * (HEAPU32[(dma >> 2) + 4]/8))
	if (pos >= 0x10000)
    buffer_pos = pos = 0;
  console.log(HEAPU32[(HEAPU32[(dma >> 2) + 7] + pos) >> 2])
  //const note = (beeps.length * Math.random()) | 0
  //let now = SND.audioContext.currentTime
  //osc.frequency.value = beeps[note]
  //parameter.setTargetAtTime(1, now, smoothingInterval)
  //parameter.setTargetAtTime(0, now + beepLengthInSeconds, smoothingInterval)
  //buffer_pos++
  
}

function SNDDMA_Shutdown() {
  if(SND.interval) {
    clearInterval(SND.interval)
  }
  SND.oscillator.disconnect(SND.gainNode);
}

function SNDDMA_BeginPainting() {
  
}

function SNDDMA_Submit() {

}

function SNDDMA_GetDMAPos() {
  if(!HEAPU32[s_soundStarted >> 2])
    return 0
  
  if ( HEAPU32[(dma >> 2) + 1] )
		samples = buffer_pos % HEAPU32[(dma >> 2) + 1];
	else
		samples = 0;

  return samples
}

function SNDDMA_Init() {
  SND.audioOutput = new Audio()
  // can't start sound until user clicks on the page
  if(INPUT.firstClick) {
    return 0
  }

  if (!SND.audioContext) {
    if (typeof(AudioContext) !== 'undefined') 
      SND.audioContext = new AudioContext();
    else if (typeof(webkitAudioContext) !== 'undefined') 
      SND.audioContext = new webkitAudioContext();
  }

  if (!SND.audioContext) throw new Error('Web Audio API is not available!')

  SND.nextPlayTime = 0
  let channelsName = stringToAddress('s_sdlChannels')
  Cvar_CheckRange( Cvar_Get( channelsName, stringToAddress('2'), 
    CVAR_ARCHIVE_ND | CVAR_LATCH ), stringToAddress('1'), 
    stringToAddress('2'), CV_INTEGER );
  SND.channels = Cvar_VariableIntegerValue(channelsName)
  Cvar_CheckRange( Cvar_Get( stringToAddress('s_sdlBits'), stringToAddress('16'), 
    CVAR_ARCHIVE_ND | CVAR_LATCH ), stringToAddress('8'), 
    stringToAddress('16'), CV_INTEGER );
  //SND.audioContext.sampleRate = Cvar_VariableIntegerValue(channelsName)

  SND.gainNode = SND.audioContext.createGain();
  SND.gainNode.connect(SND.audioContext.destination);
  SND.gainNode.gain.setValueAtTime(0, SND.audioContext.currentTime);

  SND.oscillator = SND.audioContext.createOscillator()
  SND.oscillator.frequency.value = Math.random() * 2 - 1
  if(!SND.interval) {
    SND.interval = setInterval(SNDDMA_AudioCallback_Test.bind(null, SND.gainNode.gain, SND.oscillator), 50)
  }
  SND.oscillator.connect(SND.gainNode)
  SND.oscillator.start()

  HEAPU32[(dma >> 2) + 0] /* channels */ = SND.channels
  HEAPU32[(dma >> 2) + 1] /* samples */ = SND.audioContext.sampleRate 
    * (smoothingInterval + 1000 / INPUT.fps)
  HEAPU32[(dma >> 2) + 2] /* fullsamples */ = HEAPU32[(dma >> 2) + 1] / SND.channels
  HEAPU32[(dma >> 2) + 3] /* submission_chunk */ = 1
  HEAPU32[(dma >> 2) + 4] /* samplebits */ = 16
  HEAPU32[(dma >> 2) + 5] /* isfloat */ = 1
  HEAPU32[(dma >> 2) + 6] /* speed */ = 44100
  HEAPU32[(dma >> 2) + 7] /* buffer */ = dma_buffer2
  HEAPU32[(dma >> 2) + 8] /* driver */ = stringToAddress(AudioContext.toString())

  HEAPU32[s_soundStarted >> 2] = 1
  HEAPU32[s_soundMuted >> 2] = 0
  S_Base_SoundInfo();

  return 1
}

/*
===============
SNDDMA_AudioCallback
===============
static void SNDDMA_AudioCallback(void *userdata, uint8_t *stream, int len)
{
	int pos = (dmapos * (dma.samplebits/8));
	if (pos >= dmasize)
		dmapos = pos = 0;

	if (!snd_inited)  /* shouldn't happen, but just in case...
	{
		memset(stream, '\0', len);
		return;
	}
	else
	{
		int tobufend = dmasize - pos;  /* bytes to buffer's end.
		int len1 = len;
		int len2 = 0;

		if (len1 > tobufend)
		{
			len1 = tobufend;
			len2 = len - len1;
		}
		memcpy(stream, dma.buffer + pos, len1);
		if (len2 <= 0)
			dmapos += (len1 / (dma.samplebits/8));
		else  /* wraparound?
		{
			memcpy(stream+len1, dma.buffer, len2);
			dmapos = (len2 / (dma.samplebits/8));
		}
	}

	if (dmapos >= dmasize)
		dmapos = 0;

#ifdef USE_SDL_AUDIO_CAPTURE
	if (sdlMasterGain != 1.0f)
	{
		int i;
		if (dma.isfloat && (dma.samplebits == 32))
		{
			float *ptr = (float *) stream;
			len /= sizeof (*ptr);
			for (i = 0; i < len; i++, ptr++)
			{
				*ptr *= sdlMasterGain;
			}
		}
		else if (dma.samplebits == 16)
		{
			int16_t *ptr = (int16_t *) stream;
			len /= sizeof (*ptr);
			for (i = 0; i < len; i++, ptr++)
			{
				*ptr = (int16_t) (((float) *ptr) * sdlMasterGain);
			}
		}
		else if (dma.samplebits == 8)
		{
			uint8_t *ptr = (uint8_t *) stream;
			len /= sizeof (*ptr);
			for (i = 0; i < len; i++, ptr++)
			{
				*ptr = (uint8_t) (((float) *ptr) * sdlMasterGain);
			}
		}
	}
#endif
}

static const struct
{
	uint16_t	enumFormat;
	const char	*stringFormat;
} formatToStringTable[ ] =
{
	{ AUDIO_U8,     "AUDIO_U8" },
	{ AUDIO_S8,     "AUDIO_S8" },
	{ AUDIO_U16LSB, "AUDIO_U16LSB" },
	{ AUDIO_S16LSB, "AUDIO_S16LSB" },
	{ AUDIO_U16MSB, "AUDIO_U16MSB" },
	{ AUDIO_S16MSB, "AUDIO_S16MSB" },
	{ AUDIO_F32LSB, "AUDIO_F32LSB" },
	{ AUDIO_F32MSB, "AUDIO_F32MSB" }
};

static int formatToStringTableSize = ARRAY_LEN( formatToStringTable );

*/

