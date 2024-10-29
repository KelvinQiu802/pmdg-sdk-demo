#include <stdio.h>
#include <windows.h>
#include <SimConnect.h>
#include <PMDG_NG3_SDK.h>

struct Data {
	double nose_light;
};

enum REQUEST_ID {
	REQUEST_NOSE_LIGHT,
	DATA_REQUEST
};

enum DEFINE_ID {
	DEFINE_NOSE_LIGHT
};

struct LvarData {
	double value;
};

HANDLE hSimConnect = NULL;

bool NG3_TaxiLightSwitch;
bool NG3_RunwayTurnoffSwitchLeft;
unsigned short NG3_CourseLeft;

void ProcessNG3Data(PMDG_NG3_Data* pS)
{
	// test the data access:
	// get the state of switches and save it for later use
	if (pS->LTS_TaxiSw != NG3_TaxiLightSwitch)
	{
		NG3_TaxiLightSwitch = pS->LTS_TaxiSw;
		if (NG3_TaxiLightSwitch)
			printf("TAXI LIGHTS: [ON]\n");
		else
			printf("TAXI LIGHTS: [OFF]\n");
	}

	if (pS->LTS_RunwayTurnoffSw[0] != NG3_RunwayTurnoffSwitchLeft) {
		NG3_RunwayTurnoffSwitchLeft = pS->LTS_RunwayTurnoffSw[0];
		if (NG3_RunwayTurnoffSwitchLeft)
			printf("RUNWAY TURNOFF LIGHTS LEFT: [ON]\n");
		else
			printf("RUNWAY TURNOFF LIGHTS LEFT: [OFF]\n");
	}

	if (pS->MCP_Course[0] != NG3_CourseLeft) {
		NG3_CourseLeft = pS->MCP_Course[0];
		printf("COURSE: %d\n", NG3_CourseLeft);
	}
}

void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
	SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData;
	SIMCONNECT_RECV_EXCEPTION* except;
	PMDG_NG3_Data* pS;
	switch (pData->dwID) {
	case SIMCONNECT_RECV_ID_CLIENT_DATA:
	{
		SIMCONNECT_RECV_CLIENT_DATA* pObjData =
			(SIMCONNECT_RECV_CLIENT_DATA*)pData;
		switch (pObjData->dwRequestID)
		{
		case DATA_REQUEST:
		{
			PMDG_NG3_Data* pS =
				(PMDG_NG3_Data*)&pObjData->dwData;
			ProcessNG3Data(pS);
			break;
		}
		}
		break;
	}
	default:
		printf("Unknown packet ID: %d\n", pData->dwID);
		break;
	}
}

int main() {
	HRESULT hr;

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "SimConnect Test", NULL, 0, 0, 0))) {
		printf("Connected to Flight Simulator!\n");

		hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_NG3_DATA_NAME, PMDG_NG3_DATA_ID);
		hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_NG3_DATA_DEFINITION, 0, sizeof(PMDG_NG3_Data), 0, 0);
		hr = SimConnect_RequestClientData(hSimConnect, PMDG_NG3_DATA_ID,
			DATA_REQUEST, PMDG_NG3_DATA_DEFINITION,
			SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET,
			SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);

		while (1) {
			SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);
		}

		hr = SimConnect_Close(hSimConnect);
	}
	else {
		printf("Failed to connect to Flight Simulator!\n");
	}

	return 0;
}
