#include <stdio.h>
#include <windows.h>
#include <SimConnect.h>

struct Data {
	double nose_light;
};

enum REQUEST_ID {
	REQUEST_NOSE_LIGHT
};

enum DEFINE_ID {
	DEFINE_NOSE_LIGHT
};

struct LvarData {
	double value;
};

HANDLE hSimConnect = NULL;

void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
	SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData;
	SIMCONNECT_RECV_EXCEPTION* except;
	switch (pData->dwID) {
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
		pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
		if (pObjData->dwRequestID == REQUEST_NOSE_LIGHT) {
			struct Data* pS = (struct Data*)&pObjData->dwData;
			printf("Nose Light: %.0f\n", pS->nose_light);
		}
		else {
			printf("Unknown request ID: %d\n", pObjData->dwRequestID);
		}
		break;
	case SIMCONNECT_RECV_ID_EXCEPTION:
		except = (SIMCONNECT_RECV_EXCEPTION*)pData;
		printf("Exception: %d\n", except->dwException);
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
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINE_NOSE_LIGHT, "L:S_OH_EXT_LT_NOSE", "Enum", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST_NOSE_LIGHT, DEFINE_NOSE_LIGHT, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED);

		while (1) {
			printf("Set Nose Light -> ");
			int nosepos;
			if (scanf_s("%d", &nosepos) == 1) {
				if (nosepos == -1) break;
				LvarData data = { (double)nosepos };
				hr = SimConnect_SetDataOnSimObject(hSimConnect, DEFINE_NOSE_LIGHT, SIMCONNECT_OBJECT_ID_USER, NULL, 0, sizeof(LvarData), &data);
				printf("%s\n", hr == S_OK ? "Success" : "Failed");
			}
			else {
				printf("Invalid input! Please enter a valid integer.\n");
				// Clear the input buffer  
				while (getchar() != '\n');
				continue;
			}
			SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);
		}

		hr = SimConnect_Close(hSimConnect);
	}
	else {
		printf("Failed to connect to Flight Simulator!\n");
	}

	return 0;
}
