#include "AEEModGen.h"          // Module interface definitions.
#include "AEEAppGen.h"          // Applet interface definitions.
#include "AEEShell.h"           // Shell interface definitions.  
#include "AEEHtmlViewer.h"		// For "easy" GUI
#include "AEEStdLib.h"			// We are not transphobic heathans over here, we want our standard library stuff!

#include "Cprintf_vuln_demo.h"
#include "printf_vuln_demo_res.h"
#include "htmldata.h"

typedef struct _printf_vuln_demo {
    AEEApplet  applet;
    IDisplay * piDisplay;
    IShell   * piShell;
    AEEDeviceInfo  deviceInfo;
} printf_vuln_demo;

AEEDeviceInfo device_info;
IHtmlViewer *phtmlviewer;
printf_vuln_demo * pYes;
char *html_buffer;
char *printf_buffer;
char *tmp;
int flag_value;

static  boolean printf_vuln_demo_HandleEvent(printf_vuln_demo* pMe, AEEEvent eCode,uint16 wParam, uint32 dwParam);
boolean printf_vuln_demo_InitAppData(printf_vuln_demo* pMe);
void    printf_vuln_demo_FreeAppData(printf_vuln_demo* pMe);
static void printf_vuln_demo_initstuff(printf_vuln_demo * pMe);
static void viewer_callback(void *pvUser, HViewNotify *pNotify);

int AEEClsCreateInstance(AEECLSID ClsId, IShell * piShell, IModule * piModule, void ** ppObj){
    *ppObj = NULL;
    if( AEECLSID_CPRINTF_VULN_DEMO == ClsId ) {
	    if( TRUE == AEEApplet_New(sizeof(printf_vuln_demo),
                        ClsId,
                        piShell,
                        piModule,
                        (IApplet**)ppObj,
                        (AEEHANDLER)printf_vuln_demo_HandleEvent,
                        (PFNFREEAPPDATA)printf_vuln_demo_FreeAppData) ) {
		    if(TRUE == printf_vuln_demo_InitAppData((printf_vuln_demo*)*ppObj)) {
			    return AEE_SUCCESS;
		    }
		    else {
                IApplet_Release((IApplet*)*ppObj);
                return AEE_EFAILED;
            }
        }
    }
    return AEE_EFAILED;
}

boolean printf_vuln_demo_InitAppData(printf_vuln_demo * pMe)
{
    pMe->piDisplay = pMe->applet.m_pIDisplay;
    pMe->piShell   = pMe->applet.m_pIShell;
    pMe->deviceInfo.wStructSize = sizeof(pMe->deviceInfo);
    ISHELL_GetDeviceInfo(pMe->applet.m_pIShell,&pMe->deviceInfo);
    return TRUE;
}

void printf_vuln_demo_FreeAppData(printf_vuln_demo * pMe)
{
	if(html_buffer!=NULL){
		FREE(html_buffer);
	}
	if(printf_buffer!=NULL){
		FREE(printf_buffer);
	}
	if(tmp!=NULL){
		FREE(tmp);
	}
}

static boolean printf_vuln_demo_HandleEvent(printf_vuln_demo* pMe, AEEEvent eCode, uint16 wParam, uint32 dwParam){
	if(phtmlviewer!=NULL){
		if(IHTMLVIEWER_HandleEvent(phtmlviewer,eCode,wParam,dwParam)){
			return true;
		}
	}
    switch (eCode) {
        case EVT_APP_START:
            printf_vuln_demo_initstuff(pMe);
            return TRUE;
        case EVT_APP_STOP:
      	    return TRUE;
        case EVT_APP_SUSPEND:
      	    return TRUE;
        case EVT_APP_RESUME:
            printf_vuln_demo_initstuff(pMe); 
      	    return TRUE;
        case EVT_APP_MESSAGE:
      	    return TRUE;
        case EVT_KEY:
      	    return FALSE;
        case EVT_FLIP:
            return TRUE;
        case EVT_KEYGUARD:
            return TRUE;
        default:
            break;
    }
    return FALSE;
}

static void screen_finite_state_machine(){
	SNPRINTF(html_buffer,2048,HTML_DATA,printf_buffer,flag_value);
	IHTMLVIEWER_SetData(phtmlviewer,html_buffer,-1);
}

static void submit_manager(const char *url,int *dummy){
	DBGPRINTF("flag_value address: $%p",dummy);
	//DBGPRINTF("Submit: %s",url);
	for(int i=0;i<BUFFER_LENGTH;i++){
		tmp[i]=url[i+2];
		if(url[i+2]==0){
			break;
		}
	}
	//DBGPRINTF("PRINTF: %s",tmp);
	//Now the game is cat and mouse!
	SNPRINTF(printf_buffer,BUFFER_LENGTH,tmp);
	SNPRINTF(html_buffer,2048,HTML_DATA,printf_buffer,flag_value);
	//You merely GRAZED him!
	IHTMLVIEWER_SetData(phtmlviewer,html_buffer,-1);
}

static void viewer_callback(void *pvUser, HViewNotify *pNotify){
	switch(pNotify->code){
		case HVN_NONE:
			break;
		case HVN_DONE:
			IHTMLVIEWER_Redraw(phtmlviewer);
			break;
		case HVN_JUMP:
			screen_finite_state_machine();
			IHTMLVIEWER_Redraw(phtmlviewer);
			break;
		case HVN_SUBMIT:
			submit_manager(pNotify->u.jump.pszURL,&flag_value);
			IHTMLVIEWER_Redraw(phtmlviewer);
			break;
		case HVN_FOCUS:
			break;
		case HVN_REDRAW_SCREEN:
			break;
		case HVN_INVALIDATE:
			break;
		case HVN_PAGEDONE:
			break;
		case HVN_CONTENTDONE:
			break;
		default:
			break;
	}
}

static void printf_vuln_demo_initstuff(printf_vuln_demo * pMe){
    pYes=pMe;
	html_buffer=(char*)MALLOC(sizeof(char)*2048);
	printf_buffer=(char*)MALLOC(sizeof(char)*BUFFER_LENGTH);
	tmp=(char*)MALLOC(sizeof(char)*BUFFER_LENGTH);
	flag_value=69;
	device_info.wStructSize=sizeof(AEEDeviceInfo);
	ISHELL_GetDeviceInfo(pMe->piShell,&device_info);
	IDISPLAY_ClearScreen(pMe->piDisplay);
	AEERect viewer_size;
	viewer_size.y=0;
	viewer_size.x=0;
	viewer_size.dx=device_info.cxScreen;
	viewer_size.dy=device_info.cyScreen;
	ISHELL_CreateInstance(pMe->piShell,AEECLSID_HTML,(void**)&phtmlviewer);
	IHTMLVIEWER_SetRect(phtmlviewer,&viewer_size);
	IHTMLVIEWER_SetActive(phtmlviewer,TRUE);
	screen_finite_state_machine();
	IHTMLVIEWER_SetNotifyFn(phtmlviewer,viewer_callback,pMe);
	IHTMLVIEWER_SetProperties(phtmlviewer,HVP_SCROLLBAR);
}
