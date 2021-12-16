#include "includes.h"
#include <cstdint>

//data
void* d3d9Device[119];
BYTE EndSceneBytes[7]{ 0 };
tEndScene oEndScene = nullptr;
extern LPDIRECT3DDEVICE9 pDevice = nullptr;
Hack* hack;

extern "C" __declspec(dllexport)
LRESULT wnd_hk(int32_t code, WPARAM wparam, LPARAM lparam)
{
	static auto done_once = false;

	const auto pmsg = reinterpret_cast<MSG*>(lparam);

	if (!done_once && pmsg->message == 0x5b0)
	{
		UnhookWindowsHookEx(reinterpret_cast<HHOOK>(lparam));


		done_once = true;
	}

	return CallNextHookEx(nullptr, code, wparam, lparam);
}







//hook baþlangýcý
void APIENTRY hkEndScene(LPDIRECT3DDEVICE9 o_pDevice)
{
	if (!pDevice)
		pDevice = o_pDevice;

	for (int i = 1; i < 64; i++)//entitiy döngüsü
	{
		Ent* curEnt = hack->entList->ents[i].ent;
		if (!hack->CheckValidEnt(curEnt))//sunucu tarafýndan yenileniyor mu?
			continue;
		D3DCOLOR color;

		if (curEnt->iTeamNum == hack->localEnt->iTeamNum)//takým karþýlaþtýrma
			color = D3DCOLOR_ARGB(255, 0, 255, 0);
		else
			color = D3DCOLOR_ARGB(255, 255, 0, 0);

		Vec3 entHead3D = hack->GetBonePos(curEnt, 21);//kafa kemiði pozisyonunu al
		entHead3D.z += 10;
		Vec2 entPos2D, entHead2D;
		Vec2 headbox2d;
		//snapline
		if (hack->WorldToScreen(curEnt->vecOrigin, entPos2D)) //3d entity pozisyonunu 2d ekran kordinatina çevir
		{
			//DrawLine(entPos2D.x, entPos2D.y, WindowWidth / 2, windowHeight, 2, color);//ekranýn en altýndan entity posizyona çýzgý çek
			
			if (hack->WorldToScreen(entHead3D, entHead2D))//kafa pozisyonunu 2d yap
			{
				DrawEspBox2D(entPos2D, entHead2D, 2, color);//esp kutusunu ciz
				
				//can barý
				int height = ABS(entPos2D.y - entHead2D.y);
				int dX = (entPos2D.x - entHead2D.x);

				float healthPerc = curEnt->iHealth / 100.f;//can yuzdesi


				float armorPerc = curEnt->iHealth / 100.f;//armor yüzdesi




				Vec2 botHealth, topHealth, botArmor, topArmor;
				int healthHeight = height * healthPerc;
				int armorHeight = height * armorPerc;

				botHealth.y = botArmor.y = entPos2D.y;

				botHealth.x = entPos2D.x - (height / 4) - 2;
				botArmor.x = entPos2D.x - (height / 4) + 2;

				topHealth.y = entHead2D.y + height - healthHeight;
				topArmor.y = entHead2D.y + height - armorHeight;

				topHealth.x = entPos2D.x - (height / 4) - 2 - (dX * healthPerc);
				topArmor.x = entPos2D.x - (height / 4) + 2 - (dX * armorPerc);


				DrawLine(botHealth, topHealth, 5, D3DCOLOR_ARGB(255, 46, 139, 87));
				//DrawLine(botArmor, topArmor, 2, D3DCOLOR_ARGB(255, 30, 144, 2
			}
		}
			
		if (GetAsyncKeyState(VK_LCONTROL))
		{


		}
		
	}





	//crosshair
	//DrawFilledRect(WindowWidth / 2 - 2, windowHeight / 2 - 2, 2, 2, D3DCOLOR_ARGB(255, 255, 255, 255));

	hack->crosshair2D.x = WindowWidth / 2;
	hack->crosshair2D.y = windowHeight / 2;

	Vec2 l, r, t, b;
	l = r = t = b = hack->crosshair2D;
	l.x -= hack->crosshairSize;
	r.x += hack->crosshairSize;
	b.y += hack->crosshairSize;
	t.y -= hack->crosshairSize;

	DrawLine(l, r, 10, D3DCOLOR_ARGB(255, 255, 255, 255));
	DrawLine(t, b, 10, D3DCOLOR_ARGB(255, 255, 255, 255));



	oEndScene(pDevice);
}


DWORD WINAPI HackThread(HMODULE hModule)
{
	//hook
	if (GetD3D9Device(d3d9Device, sizeof(d3d9Device)))
	{
		memcpy(EndSceneBytes, (char*)d3d9Device[42], 7);

		oEndScene = (tEndScene)TrampHook((char*)d3d9Device[42], (char*)hkEndScene, 7);
	}

	hack = new Hack();
	hack->Init();

	//hack loop
	while (!GetAsyncKeyState(VK_END))
	{
		hack->Update();
		//Vec3 pAng = hack->localEnt->aimPunchAngle;
	//		hack->crosshair2D.y= WindowWidth / 2 - (WindowWidth / 90 * pAng.y);
	//	hack->crosshair2D.x = windowHeight / 2 - (windowHeight / 90 * pAng.x);

	}


	//unhook
	Patch((BYTE*)d3d9Device[42], EndSceneBytes, 7);

	//uninject
	FreeLibraryAndExitThread(hModule, 0);
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpr)
{
	if (reason == DLL_PROCESS_ATTACH)
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, 0));
	return TRUE;

}