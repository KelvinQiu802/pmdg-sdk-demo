#include <stdio.h>
#include <windows.h>
#include <SimConnect.h>

struct Data {
	double taxi_light;
};

enum EVENT_ID {
	EVENT_TAXI_LIGHT_TOGGLE,
};

enum REQUEST_ID {
	REQUEST_TAXI_LIGHT
};

enum DEFINE_ID {
	DEFINE_TAXI_LIGHT
};

enum GROUP_ID {
	HIGHEST_PRIORITY_GROUP
};

HANDLE hSimConnect = NULL;

void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
	SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData;
	SIMCONNECT_RECV_EVENT* evt;
	SIMCONNECT_RECV_OPEN* openData;
	switch (pData->dwID) {
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
		pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
		if (pObjData->dwRequestID == REQUEST_TAXI_LIGHT) {
			struct Data* pS = (struct Data*)&pObjData->dwData;
			printf("Taxi Light: %s\n", pS->taxi_light == 1.0 ? "On" : "Off");
		}
		else {
			printf("Unknown request ID: %d\n", pObjData->dwRequestID);
		}
		break;
	case SIMCONNECT_RECV_ID_EVENT:
		evt = (SIMCONNECT_RECV_EVENT*)pData;
		if (evt->uEventID == EVENT_TAXI_LIGHT_TOGGLE) {
			// Toggle Taxi Light
			break;
		}
		break;
	case SIMCONNECT_RECV_ID_OPEN:
		openData = (SIMCONNECT_RECV_OPEN*)pData;
		printf("Connected to %s\n", openData->szApplicationName);
		break;
	default:
		printf("Unknown packet ID: %d\n", pData->dwID);
		break;
	}
}

int main() {
	HRESULT hr;

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "SimConnect Test", NULL, 0, 0, 0))) {
		printf("Connected to Flight Simulator!\n");

		// Object Data
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_TAXI_LIGHT, "LIGHT TAXI", "Bool");
		
		// Event
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_TAXI_LIGHT_TOGGLE, "TOGGLE_TAXI_LIGHTS");
		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, HIGHEST_PRIORITY_GROUP, EVENT_TAXI_LIGHT_TOGGLE);
		hr = SimConnect_SetNotificationGroupPriority(hSimConnect, HIGHEST_PRIORITY_GROUP, SIMCONNECT_GROUP_PRIORITY_HIGHEST);


		while (1) {
			printf("Press 'q' to quit, '1' to toggle taxi light: ");
			char c = getchar();
			getchar();
			if (c == '1') {
				hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST_TAXI_LIGHT, DEFINE_TAXI_LIGHT, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_ONCE);
				hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, EVENT_TAXI_LIGHT_TOGGLE, 0, HIGHEST_PRIORITY_GROUP, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);
			}
			if (c == 'q' || c == 'Q') {
				break;
			}
		}

		hr = SimConnect_Close(hSimConnect);
	}
	else {
		printf("Failed to connect to Flight Simulator!\n");
	}

	return 0;
}
