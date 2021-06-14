
#ifdef USE_LNBITS
#ifndef __WASM__
void SV_CheckInvoiceStatus(invoice_t *updateInvoice) {
	// extract key
	int i;
	int r = 0, ki = 0, vi = 0;
	char keyName[64];
	char paidValue[10];
	qboolean key = qfalse, value = qfalse;
	qboolean paymentValue = qfalse, checkingId = qfalse, withdraw = qfalse;
	qboolean paid = qfalse;
	if(!svDownload.TempStore[0]) return;

	r = strlen(svDownload.TempStore);
	// simple state machine for checking payment status json
	for(i = 0; i < r; i++) {
		if(!key && !value && svDownload.TempStore[i] == '"') {
			if(paymentValue || checkingId || withdraw) {
				value = qtrue;
				vi = 0;
			} else {
				key = qtrue;
				ki = 0;
			}
		} else if (key && svDownload.TempStore[i] == '"') {
			key = qfalse;
			keyName[ki] = '\0';
			paymentValue = !Q_stricmp(keyName, "payment_request");
			checkingId = !Q_stricmp(keyName, "checking_id");
			withdraw = !Q_stricmp(keyName, "lnurl");
			paid = !Q_stricmp(keyName, "paid");
			vi = 0;
		} else if (value && svDownload.TempStore[i] == '"') {
			value = qfalse;
			paymentValue = 
			checkingId = 
			withdraw = qfalse;
		} else if (key) {
			if(ki < 63) {
				keyName[ki++] = svDownload.TempStore[i];
			}
		} else if (value) {
			if(vi < 63 && checkingId) {
				updateInvoice->checkingId[vi++] = svDownload.TempStore[i];
			} else if (vi < 255 && paymentValue) {
				updateInvoice->invoice[vi++] = svDownload.TempStore[i];
			} else if (vi < 255 && withdraw)
				updateInvoice->reward[vi++] = svDownload.TempStore[i];
		} else if (paid && svDownload.TempStore[i] == '}') {
			if(Q_stristr(paidValue, "true")) {
				updateInvoice->paid = qtrue;
			} else if (Q_stristr(paidValue, "false")) {
				updateInvoice->paid = qfalse;
			}
		} else if (paid) {
			if(vi < 9) {
				paidValue[vi++] = svDownload.TempStore[i];
			}
		}
	}
	svDownload.TempStore[0] = '\0';
}
#endif


void SV_SendInvoiceAndChallenge(const netadr_t *from, char *invoiceData, char *reward, const char *oldChallenge) {
	int			challenge;
	char	infostring[MAX_INFO_STRING];
	infostring[0] = '\0';
	Info_SetValueForKey( infostring, "cl_lnInvoice", invoiceData );
	Info_SetValueForKey( infostring, "oldChallenge", oldChallenge );
	challenge = SV_CreateChallenge( svs.time >> TS_SHIFT, from );
	if(reward[0]) {
		Info_SetValueForKey( infostring, "reward", reward );
	}
	Info_SetValueForKey( infostring, "challenge", va("%i", challenge) );
	NET_OutOfBandPrint( NS_SERVER, from, "infoResponse\n%s", infostring );
}


void SV_CheckInvoicesAndPayments( void ) {
	int i, now, highestScore = 0;
	client_t	*highestClient;
	playerState_t	*ps;
	invoice_t  *oldestInvoice = NULL;
	int        oldestInvoiceTime;
	if(!maxInvoices) return;

	// don't even bother if a request is already in progress
#ifdef __WASM__
	if(svDownload)
#else
	if(svDownload.cURL)
#endif
		return;

	now = Sys_Milliseconds();
	oldestInvoiceTime = now;
	oldestInvoice = NULL;
	for(i=0;i<(sv_maxclients->integer+10);i++) {

		if(maxInvoices[i].guid[0] && maxInvoices[i].lastTime < oldestInvoiceTime
			&& !maxInvoices[i].paid && !highestScore
		  && (now - maxInvoices[i].lastTime) > 1000) {
			oldestInvoiceTime = maxInvoices[i].lastTime;
			oldestInvoice = &maxInvoices[i];
		}

		if(!maxInvoices[i].cl) continue;

		// detect client with highest score to reward
		ps = SV_GameClientNum( maxInvoices[i].cl - svs.clients);
		if (ps->pm_type == PM_INTERMISSION
			&& ps->persistant[PERS_SCORE] > highestScore) {
			highestClient = maxInvoices[i].cl;
			highestScore = ps->persistant[PERS_SCORE];
			oldestInvoice = &maxInvoices[i];
		}
	}

	if(requestInvoice) {
		qboolean wasntPaid = !requestInvoice->paid;
		// update the oldest invoice before finding a new one
#ifndef __WASM__
		if(svDownload.TempStore[0]) {
			SV_CheckInvoiceStatus(requestInvoice);
		}
#endif
		if(wasntPaid && requestInvoice->paid) {
			// add this once when paid status changes
			if(sv_lnMatchCut->integer < requestInvoice->price) {
				Cvar_Set("sv_lnMatchReward", va("%i", sv_lnMatchReward->integer
					+ requestInvoice->price - sv_lnMatchCut->integer));
			}
		}
		requestInvoice = NULL;
	}

	if(!oldestInvoice)
		return;
	oldestInvoice->lastTime = now;

	if(!oldestInvoice->invoice[0]) {
		// create the invoice
		Com_sprintf( invoicePostData, sizeof( invoicePostData ),
			"{\"out\": false, \"amount\": %i, \"memo\": \"%s\"}",
			oldestInvoice->price, oldestInvoice->guid );
		Com_sprintf( invoicePostHeaders, sizeof( invoicePostHeaders ),
			"X-Api-Key: %s", sv_lnWallet->string );
		Com_sprintf( &invoicePostHeaders[strlen( invoicePostHeaders ) + 1],
			sizeof( invoicePostHeaders ) - strlen( invoicePostHeaders ) - 1,
			"Content-type: application/json");
		requestInvoice = oldestInvoice;
#ifdef __WASM__
		svDownload = qtrue;
		//Sys_BeginDownload();
#else
		svDownload.isPost = qtrue;
		svDownload.isPak = qfalse;
		svDownload.headers.readptr = invoicePostHeaders;
		svDownload.headers.sizeleft = sizeof(invoicePostHeaders);
		svDownload.headers.dl = &svDownload;
		svDownload.data.readptr = invoicePostData;
		svDownload.data.sizeleft = strlen(invoicePostData);
		svDownload.data.dl = &svDownload;
		Com_DL_BeginPost(&svDownload, "", va("%s/payments", sv_lnAPI->string));
#endif
} else if (!oldestInvoice->paid && oldestInvoice->checkingId[0]) {
		// check for payment
		Com_sprintf( invoicePostHeaders, sizeof( invoicePostHeaders ),
			"X-Api-Key: %s", sv_lnWallet->string );
		Com_sprintf( &invoicePostHeaders[strlen( invoicePostHeaders ) + 1],
			sizeof( invoicePostHeaders ) - strlen( invoicePostHeaders ) - 1,
			"Content-type: application/json");
		requestInvoice = oldestInvoice;
#ifdef __WASM__
		svDownload = qtrue;
		//Sys_BeginDownload();
#else
		svDownload.isPost = qfalse;
		svDownload.isPak = qfalse;
		svDownload.headers.readptr = invoicePostHeaders;
		svDownload.headers.sizeleft = sizeof(invoicePostHeaders);
		svDownload.headers.dl = &svDownload;
		Com_DL_BeginPost(&svDownload, "",
			va("%s/payments/%s", sv_lnAPI->string, oldestInvoice->checkingId));
#endif
	} else if (highestClient && highestScore > 0 && sv_lnMatchReward->integer > 0) {
		// send the reward to the client
		if(oldestInvoice->reward[0]) {
			SV_SendInvoiceAndChallenge(&highestClient->netchan.remoteAddress, oldestInvoice->invoice,
				oldestInvoice->reward, va("%i", highestClient->challenge));
			return;
		}
		Com_sprintf( invoicePostData, sizeof( invoicePostData ),
			"%s %s %i, %s %i, %s",
			"{\"title\": \"QuakeJS winner\",",
			"\"min_withdrawable\":", sv_lnMatchReward->integer,
			"\"max_withdrawable\":", sv_lnMatchReward->integer,
			"\"uses\": 1, \"wait_time\": 1, \"is_unique\": true}");
		Com_sprintf( invoicePostHeaders, sizeof( invoicePostHeaders ),
			"X-Api-Key: %s", sv_lnKey->string );
		Com_sprintf( &invoicePostHeaders[strlen( invoicePostHeaders ) + 1],
			sizeof( invoicePostHeaders ) - strlen( invoicePostHeaders ) - 1,
			"Content-type: application/json");
		requestInvoice = oldestInvoice;
#ifdef __WASM__
		svDownload = qtrue;
		//Sys_BeginDownload();
#else
		svDownload.isPost = qtrue;
		svDownload.isPak = qfalse;
		svDownload.headers.readptr = invoicePostHeaders;
		svDownload.headers.sizeleft = sizeof(invoicePostHeaders);
		svDownload.headers.dl = &svDownload;
		svDownload.data.readptr = invoicePostData;
		svDownload.data.sizeleft = strlen(invoicePostData);
		svDownload.data.dl = &svDownload;
		Com_DL_BeginPost(&svDownload, "", va("%s/links", sv_lnWithdraw->string));
#endif			
	}
}

invoice_t *SVC_ClientRequiresInvoice(const netadr_t *from, const char *userinfo, int challenge) {
	int i;
	char *cl_invoice = Info_ValueForKey(userinfo, "cl_lnInvoice");
	char *cl_guid = Info_ValueForKey(userinfo, "cl_guid");
	invoice_t *found = NULL;
	
	// perform curl request to get invoice id
	for(i=0;i<sv_maxclients->integer+10;i++) {
		if(cl_guid[0] && maxInvoices[i].guid[0]
			&& !Q_stricmp(maxInvoices[i].guid, cl_guid)) {
			found = &maxInvoices[i];
			break;
		}
	}

	if(!cl_invoice[0] || !found) {
		if(!found) {
			NET_OutOfBandPrint( NS_SERVER, from, "print\n402: PAYMENT REQUIRED\n" );
			Com_Printf( "Payment required for new client: %s.\n", cl_guid );
			memset(&maxInvoices[numInvoices], 0, sizeof(invoice_t));
			strcpy(maxInvoices[numInvoices].guid, cl_guid);
			maxInvoices[numInvoices].price = sv_lnMatchPrice->integer;
			numInvoices++;
			if(numInvoices > sv_maxclients->integer+10) {
				numInvoices = 0;
			}
		} else if (!found->invoice[0]) {
			NET_OutOfBandPrint( NS_SERVER, from, "print\n402: PAYMENT REQUIRED (invoicing...)\n" );
		} else if (found) {
			NET_OutOfBandPrint( NS_SERVER, from, "print\n402: PAYMENT REQUIRED\n" );
			SV_SendInvoiceAndChallenge(from, found->invoice, "", va("%i", challenge));
		}
		return NULL;
	} else if (found && !maxInvoices[i].paid) {
		NET_OutOfBandPrint( NS_SERVER, from, "print\n402: PAYMENT REQUIRED\n" );
		Com_Printf( "Checking payment for known client: %s (%s).\n", cl_guid, &found->invoice[strlen(found->invoice) - 8] );
		SV_SendInvoiceAndChallenge(from, found->invoice, "", va("%i", challenge));
		return NULL;
	} else if (found) {
		// client has paid, let them through
		return found;
	}
	return NULL;
}

#endif
