/*
*******************************************************************************
\file btok_test.c
\brief Tests for STB 34.101.79 (btok)
\project bee2/test
\created 2022.07.07
\version 2022.11.10
\license This program is released under the GNU General Public License 
version 3. See Copyright Notices in bee2/info.h.
*******************************************************************************
*/

#include <bee2/core/err.h>
#include <bee2/core/hex.h>
#include <bee2/core/mem.h>
#include <bee2/core/prng.h>
#include <bee2/core/str.h>
#include <bee2/core/util.h>
#include <bee2/crypto/belt.h>
#include <bee2/crypto/bign.h>
#include <bee2/crypto/btok.h>

/*
*******************************************************************************
CV-сертификаты
*******************************************************************************
*/

static bool_t btokCVCTest()
{
	octet echo[256];
	btok_cvc_t cvc0[1];
	btok_cvc_t cvc1[1];
	btok_cvc_t cvc2[1];
	bign_params params[1];
	octet privkey0[64];
	octet privkey1[48];
	octet privkey2[32];
	octet cert0[400]; size_t cert0_len;
	octet cert1[400]; size_t cert1_len;
	octet cert2[400]; size_t cert2_len;
	// запустить ГПСЧ
	prngEchoStart(echo, beltH(), 256);
	// определить максимальную длину сертификата
	memSetZero(cvc0, sizeof(btok_cvc_t));
	strCopy(cvc0->authority, "BYCA00000000");
	strCopy(cvc0->holder, "BYCA00000000");
	hexTo(cvc0->from, "020200070007");
	hexTo(cvc0->until, "090900070007");
	memSet(cvc0->hat_eid, 0xEE, sizeof(cvc0->hat_eid));
	memSet(cvc0->hat_esign, 0x77, sizeof(cvc0->hat_esign));
	cvc0->pubkey_len = 128;
	if (btokCVCCheck(cvc0) == ERR_OK)
		return FALSE;
	if (bignStdParams(params, "1.2.112.0.2.0.34.101.45.3.3") != ERR_OK ||
		bignGenKeypair(privkey0, cvc0->pubkey, params, prngEchoStepR,
			echo) != ERR_OK ||
		btokCVCCheck(cvc0) != ERR_OK)
		return FALSE;
	if (btokCVCWrap(0, 0, cvc0, privkey0, 64) != ERR_OK)
		return FALSE;
	cvc0->pubkey_len = 0;
	if (btokCVCWrap(0, &cert0_len, cvc0, privkey0, 64) != ERR_OK)
		return FALSE;
	ASSERT(cert0_len == 365);
	// выпустить cert0
	memSetZero(cvc0->authority, sizeof(cvc0->authority));
	strCopy(cvc0->authority, "BYCA0000");
	memSetZero(cvc0->holder, sizeof(cvc0->holder));
	strCopy(cvc0->holder, "BYCA0000");
	if (btokCVCWrap(cert0, &cert0_len, cvc0, privkey0, 64) != ERR_OK)
		return FALSE;
	ASSERT(cert0_len < 365);
	// разобрать cert0
	if (btokCVCUnwrap(cvc1, cert0, cert0_len, 0, 0) != ERR_OK ||
		btokCVCUnwrap(cvc1, cert0, cert0_len, cvc0->pubkey,
			cvc0->pubkey_len) != ERR_OK ||
		!memEq(cvc0, cvc1, sizeof(btok_cvc_t)) ||
		btokCVCLen(cert0, cert0_len) != cert0_len ||
		btokCVCLen(cert0, cert0_len + 1) != cert0_len ||
		btokCVCLen(cert0, cert0_len - 1) != SIZE_MAX ||
		btokCVCMatch(cert0, cert0_len, privkey0, 64) != ERR_OK)
		return FALSE;
	// составить и проверить cvc1
	memSetZero(cvc1, sizeof(btok_cvc_t));
	strCopy(cvc1->authority, "BYCA0000");
	strCopy(cvc1->holder, "BYCA1000");
	hexTo(cvc1->from, "020200070102");
	hexTo(cvc1->until, "020201010300");
	memSet(cvc1->hat_eid, 0xDD, sizeof(cvc1->hat_eid));
	memSet(cvc1->hat_esign, 0x33, sizeof(cvc1->hat_esign));
	cvc1->pubkey_len = 96;
	if (bignStdParams(params, "1.2.112.0.2.0.34.101.45.3.2") != ERR_OK ||
		bignGenKeypair(privkey1, cvc1->pubkey, params, prngEchoStepR,
			echo) != ERR_OK ||
		btokCVCCheck(cvc1) != ERR_OK)
		return FALSE;
	// создать pre-cert1 (запрос на выпуск сертификата)
	if (btokCVCWrap(0, &cert1_len, cvc1, privkey1, 48) != ERR_OK)
		return FALSE;
	ASSERT(cert1_len <= sizeof(cert1));
	if (btokCVCWrap(cert1, 0, cvc1, privkey1, 48) != ERR_OK)
		return FALSE;
	// разобрать pre-cert1:
	// - извлечь открытый ключ,
	// - проверить подпись,
	// - проверить соответствие authority <=> holder
	if (btokCVCUnwrap(cvc1, cert1, cert1_len, 0, 0) != ERR_OK ||
		btokCVCUnwrap(cvc2, cert1, cert1_len, cvc1->pubkey,
			cvc1->pubkey_len) != ERR_OK ||
		!memEq(cvc1, cvc2, sizeof(btok_cvc_t)) ||
		!strEq(cvc1->authority, cvc0->holder))
		return FALSE;
	// создать cert1
	if (btokCVCWrap(0, &cert1_len, cvc1, privkey0, 64) != ERR_OK)
		return FALSE;
	ASSERT(cert1_len <= sizeof(cert1));
	if (btokCVCWrap(cert1, &cert1_len, cvc1, privkey0, 64) != ERR_OK)
		return FALSE;
	// составить cvc2
	memSetZero(cvc2, sizeof(btok_cvc_t));
	strCopy(cvc2->authority, "BYCA1000");
	strCopy(cvc2->holder, "590082394654");
	hexTo(cvc2->from, "020200070102");
	hexTo(cvc2->until, "030901020301");
	memSet(cvc2->hat_eid, 0x88, sizeof(cvc2->hat_eid));
	memSet(cvc2->hat_esign, 0x11, sizeof(cvc2->hat_esign));
	cvc2->pubkey_len = 64;
	if (bignStdParams(params, "1.2.112.0.2.0.34.101.45.3.1") != ERR_OK ||
		bignGenKeypair(privkey2, cvc2->pubkey, params, prngEchoStepR,
			echo) != ERR_OK ||
		btokCVCCheck(cvc2) != ERR_OK)
		return FALSE;
	// выпустить cert2
	if (btokCVCIss(cert2, &cert2_len, cvc2, cert1, cert1_len - 1,
			privkey1, 48) == ERR_OK ||
		btokCVCIss(cert2, &cert2_len, cvc2, cert1, cert1_len,
			privkey1, 48 + 1) == ERR_OK ||
		btokCVCIss(cert2, &cert2_len, cvc2, cert1, cert1_len,
			privkey1, 48) != ERR_OK)
		return FALSE;
	ASSERT(cert2_len <= sizeof(cert2));
	// проверить сертификаты
	if (btokCVCVal(cert1, cert1_len, cert0, cert0_len, 0) != ERR_OK ||
		btokCVCVal(cert2, cert2_len, cert1, cert1_len, 0) != ERR_OK ||
		btokCVCVal(cert2, cert2_len, cert1, cert1_len, cvc0->from) == ERR_OK ||
		btokCVCVal2(cvc1, cert1, cert1_len, cvc0, 0) != ERR_OK ||
		btokCVCVal2(cvc2, cert2, cert2_len, cvc1, 0) != ERR_OK ||
		btokCVCVal2(cvc2, cert2, cert2_len, cvc1, cvc0->until) == ERR_OK)
		return FALSE;
	// все хорошо
	return TRUE;
}

/*
*******************************************************************************
SM
*******************************************************************************
*/

static bool_t btokSMTest()
{
	octet state_t[512];
	octet state_ct[512];
	octet stack[4096];
	apdu_cmd_t* cmd = (apdu_cmd_t*)stack;
	apdu_cmd_t* cmd1 = (apdu_cmd_t*)(stack + 1024);
	apdu_resp_t* resp = (apdu_resp_t*)(stack + 2 * 1024);
	apdu_resp_t* resp1 = (apdu_resp_t*)(stack + 3 * 1024);
	octet apdu[1024];
	size_t count;
	size_t size;
	// запустить SM
	ASSERT(btokSM_keep() <= sizeof(state_t));
	ASSERT(btokSM_keep() <= sizeof(state_ct));
	btokSMStart(state_t, beltH());
	btokSMStart(state_ct, beltH());
	// обработка команды без защиты
	memSetZero(cmd, sizeof(apdu_cmd_t));
	cmd->cla = 0x00, cmd->ins = 0xA4, cmd->p1 = 0x04, cmd->p2 = 0x04;
	cmd->cdf_len = 4, cmd->rdf_len = 256;
	hexTo(cmd->cdf, "54657374");
	if (btokSMCmdWrap(0, 0, cmd, 0) != ERR_OK ||
		btokSMCmdWrap(0, &count, cmd, 0) != ERR_OK ||
		count != 10 ||
		btokSMCmdWrap(apdu, 0, cmd, 0) != ERR_OK ||
		btokSMCmdWrap(apdu, &count, cmd, 0) != ERR_OK ||
		count != 10 ||
		!hexEq(apdu, "00A40404045465737400") ||
		btokSMCmdUnwrap(0, 0, apdu, count, 0) != ERR_OK ||
		btokSMCmdUnwrap(0, &size, apdu, count, 0) != ERR_OK ||
		size != sizeof(apdu_cmd_t) + 4 ||
		btokSMCmdUnwrap(cmd1, 0, apdu, count, 0) != ERR_OK ||
		btokSMCmdUnwrap(cmd1, &size, apdu, count, 0) != ERR_OK ||
		size != sizeof(apdu_cmd_t) + 4 ||
		!memEq(cmd, cmd1, size))
		return FALSE;
	// обработка ответа без защиты
	memSetZero(resp, sizeof(apdu_resp_t));
	resp->sw1 = 0x90, resp->sw2 = 0x00;
	resp->rdf_len = 20;
	hexTo(resp->rdf, "E012C00401FF8010C00402FF8010C00403FF8010");
	if (btokSMRespWrap(0, 0, resp, 0) != ERR_OK ||
		btokSMRespWrap(0, &count, resp, 0) != ERR_OK ||
		count != 22 ||
		btokSMRespWrap(apdu, 0, resp, 0) != ERR_OK ||
		btokSMRespWrap(apdu, &count, resp, 0) != ERR_OK ||
		count != 22 ||
		!hexEq(apdu,
			"E012C00401FF8010C00402FF8010C00403FF80109000") ||
		btokSMRespUnwrap(0, 0, apdu, count, 0) != ERR_OK ||
		btokSMRespUnwrap(0, &size, apdu, count, 0) != ERR_OK ||
		size != sizeof(apdu_resp_t) + 20 ||
		btokSMRespUnwrap(resp1, 0, apdu, count, 0) != ERR_OK ||
		btokSMRespUnwrap(resp1, &size, apdu, count, 0) != ERR_OK ||
		size != sizeof(apdu_resp_t) + 20 ||
		!memEq(resp, resp1, size))
		return FALSE;
	// обработка команды с защитой
	btokSMCtrInc(state_t), btokSMCtrInc(state_ct);
	if (btokSMCmdWrap(0, &count, cmd, state_t) != ERR_OK ||
		count != 26 ||
		btokSMCmdWrap(apdu, &count, cmd, state_t) != ERR_OK ||
		count != 26 ||
		!hexEq(apdu,
			"04A4040414"
			"8705020C4C0BFB"
			"970100"
			"8E08FBD50AD6814F90A9"
			"00") ||
		btokSMCmdUnwrap(0, &size, apdu, count, state_ct) != ERR_OK ||
		size != sizeof(apdu_cmd_t) + 4 ||
		btokSMCmdUnwrap(cmd1, &size, apdu, count, state_ct) != ERR_OK ||
		size != sizeof(apdu_cmd_t) + 4 ||
		!memEq(cmd, cmd1, size))
		return FALSE;
	// обработка ответа с защитой
	btokSMCtrInc(state_ct), btokSMCtrInc(state_t);
	if (btokSMRespWrap(0, &count, resp, state_ct) != ERR_OK ||
		count != 35 ||
		btokSMRespWrap(apdu, &count, resp, state_ct) != ERR_OK ||
		count != 35 ||
		!hexEq(apdu,
			"8715022A9042A60A85E50FAB446AC80B75F144B67EBD6D"
			"8E082C4DE31DFAA17635"
			"9000") ||
		btokSMRespUnwrap(0, &size, apdu, count, state_t) != ERR_OK ||
		size != sizeof(apdu_resp_t) + 20 ||
		btokSMRespUnwrap(resp1, &size, apdu, count, state_t) != ERR_OK ||
		size != sizeof(apdu_resp_t) + 20 ||
		!memEq(resp, resp1, size))
		return FALSE;
	// защита команд и ответов: сочетания длин
	cmd->cla = 0x00, cmd->ins = 0xA4, cmd->p1 = 0x04, cmd->p2 = 0x04;
	for (cmd->cdf_len = 0; cmd->cdf_len <= 257; ++cmd->cdf_len)
		for (cmd->rdf_len = 0; cmd->rdf_len <= 257; ++cmd->rdf_len)
		{
			btokSMCtrInc(state_t);
			if (btokSMCmdWrap(apdu, &count, cmd, state_t) != ERR_OK)
				return FALSE;
			ASSERT(count < sizeof(apdu));
			btokSMCtrInc(state_ct);
			if (btokSMCmdUnwrap(cmd1, &size, apdu, count, state_ct) != ERR_OK)
				return FALSE;
			ASSERT(size < sizeof(stack) / 4);
			if (!memEq(cmd, cmd1, size))
				return FALSE;
			resp->rdf_len = cmd->rdf_len;
			btokSMCtrInc(state_ct);
			if (btokSMRespWrap(apdu, &count, resp, state_ct) != ERR_OK)
				return FALSE;
			ASSERT(count < sizeof(apdu));
			btokSMCtrInc(state_t);
			if (btokSMRespUnwrap(resp1, &size, apdu, count, state_t) != ERR_OK)
				return FALSE;
			ASSERT(size < sizeof(stack) / 4);
			if (!memEq(resp, resp1, size))
				return FALSE;
		}
	// все хорошо
	return TRUE;
}

/*
*******************************************************************************
BAUTH

A = T, B = CT
*******************************************************************************
*/

static const char _da[] =
	"1F66B5B84B7339674533F0329C74F218"
	"34281FED0732429E0C79235FC273E269";

static const char _db[] =
	"4C0E74B2CD5811AD21F23DE7E0FA742C"
	"3ED6EC483C461CE15C33A77AA308B7D2";

static const char _certa[] =
	"5430303030303031"
	"BD1A5650179D79E03FCEE49D4C2BD5DD"
	"F54CE46D0CF11E4FF87BF7A890857FD0"
	"7AC6A60361E8C8173491686D461B2826"
	"190C2EDA5909054A9AB84D2AB9D99A90";

static const char _certb[] =
	"4354303030303031"
	"CCEEF1A313A406649D15DA0A851D486A"
	"695B641B20611776252FFDCE39C71060"
	"7C9EA1F33C23D20DFCB8485A88BE6523"
	"A28ECC3215B47FA289D6C9BE1CE837C0";

static err_t bakeTestCertVal(octet* pubkey, const bign_params* params,
	const octet* data, size_t len)
{
	if (!memIsValid(params, sizeof(bign_params)) ||
		(params->l != 128 && params->l != 192 && params->l != 256) ||
		!memIsNullOrValid(pubkey, params->l / 2))
		return ERR_BAD_INPUT;
	if (!memIsValid(data, len) ||
		len < params->l / 2)
		return ERR_BAD_CERT;
	if (pubkey)
		memCopy(pubkey, data + (len - params->l / 2), params->l / 2);
	return ERR_OK;
}

bool_t btokBAUTHTest() 
{
	bign_params params[1];
	octet echoa[64];
	octet echob[64];
	bake_settings settingsa[1];
	bake_settings settingsb[1];
	octet da[32];
	octet db[32];
	octet certdataa[8 /* Т0000001 */ + 64];
	octet certdatab[8 /* СТ000001 */ + 64];
	bake_cert certa[1];
	bake_cert certb[1];
	octet keya[32];
	octet keyb[32];
	octet statea[20000];
	octet stateb[20000];
	octet buf[1000];
	// загрузить долговременные параметры
	if (bignStdParams(params, "1.2.112.0.2.0.34.101.45.3.1") != ERR_OK)
		return FALSE;
	// настроить генераторы
	ASSERT(prngEcho_keep() <= sizeof(echoa));
	// загрузить личные ключи
	hexTo(da, _da);
	hexTo(db, _db);
	// загрузить сертификаты
	hexTo(certdataa, _certa);
	hexTo(certdatab, _certb);
	certa->data = certdataa;
	certa->len = strLen(_certa) / 2;
	certb->data = certdatab;
	certb->len = strLen(_certb) / 2;
	certa->val = certb->val = bakeTestCertVal;
	// очистка
	memSetZero(statea, sizeof(statea));
	memSetZero(stateb, sizeof(stateb));
	memSetZero(keya, sizeof(keya));
	memSetZero(keyb, sizeof(keyb));
	memSetZero(buf, sizeof(buf));
	// задать настройки, с аутентификацией КТ
	memSetZero(settingsa, sizeof(bake_settings));
	memSetZero(settingsb, sizeof(bake_settings));
	settingsa->kca = settingsb->kca = TRUE;
	settingsa->kcb = settingsb->kcb = TRUE;
	settingsa->rng = settingsb->rng = prngEchoStepR;
	settingsa->rng_state = echoa;
	settingsb->rng_state = echob;
	prngEchoStart(echoa, beltH(), 128);
	prngEchoStart(echob, beltH() + 128, 128);
	// инициализация
	ASSERT(btokBAuthT_keep(params->l) <= sizeof(statea));
	ASSERT(btokBAuthCT_keep(params->l) <= sizeof(stateb));
	if (btokBAuthTStart(statea, params, settingsa, da, certa) != ERR_OK ||
		btokBAuthCTStart(stateb, params, settingsb, db, certb) != ERR_OK)
		return FALSE;
	// шаги протокола, с аутентификацией КТ
	if (btokBAuthCTStep2(buf, certa, stateb) != ERR_OK ||
		btokBAuthTStep3(buf, buf, statea) != ERR_OK ||
		btokBAuthCTStep4(buf, buf, stateb) != ERR_OK ||
		btokBAuthTStep5(buf, 8 + 32 + certb->len,
			bakeTestCertVal, statea) != ERR_OK)
		return FALSE;
	// извлечение ключей
	if (btokBAuthCTStepG(keyb, stateb) != ERR_OK ||
		btokBAuthTStepG(keya, statea) != ERR_OK ||
		!memEq(keya, keyb, 32))
		return FALSE;
	// очистка
	memSetZero(statea, sizeof(statea));
	memSetZero(stateb, sizeof(stateb));
	memSetZero(keya, sizeof(keya));
	memSetZero(keyb, sizeof(keyb));
	memSetZero(buf, sizeof(buf));
	// задать настройки, без аутентификациии КТ
	memSetZero(settingsa, sizeof(bake_settings));
	memSetZero(settingsb, sizeof(bake_settings));
	settingsa->kca = settingsb->kca = TRUE;
	settingsa->kcb = settingsb->kcb = FALSE;
	settingsa->rng = settingsb->rng = prngEchoStepR;
	settingsa->rng_state = echoa;
	settingsb->rng_state = echob;
	prngEchoStart(echoa, beltH(), 128);
	prngEchoStart(echob, beltH() + 128, 128);
	// инициализация
	ASSERT(btokBAuthT_keep(params->l) <= sizeof(statea));
	ASSERT(btokBAuthCT_keep(params->l) <= sizeof(stateb));
	if (btokBAuthTStart(statea, params, settingsa, da, certa) != ERR_OK ||
		btokBAuthCTStart(stateb, params, settingsb, db, certb) != ERR_OK)
		return FALSE;
	// шаги протокола, без аутентификациии КТ
	if (btokBAuthCTStep2(buf, certa, stateb) != ERR_OK ||
		btokBAuthTStep3(buf, buf, statea) != ERR_OK ||
		btokBAuthCTStep4(buf, buf, stateb) != ERR_OK)
		return FALSE;
	// извлечение ключей
	if (btokBAuthCTStepG(keyb, stateb) != ERR_OK ||
		btokBAuthTStepG(keya, statea) != ERR_OK ||
		!memEq(keya, keyb, 32))
		return FALSE;
	// все нормально
	return TRUE;
}

bool_t btokTest()
{
	return btokCVCTest() && btokSMTest() && btokBAUTHTest();
}