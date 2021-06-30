/*
*******************************************************************************
\file ec2.h
\brief Elliptic curves over binary fields
\project bee2 [cryptographic library]
\author (C) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2012.04.19
\version 2021.06.30
\license This program is released under the GNU General Public License 
version 3. See Copyright Notices in bee2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file ec2.h
\brief Эллиптические кривые над двоичными полями
*******************************************************************************
*/

#ifndef __BEE2_EC2_H
#define __BEE2_EC2_H

#include "bee2/math/ec.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\file ec2.h

Реализована арифметика эллиптических кривых над полем f = GF(2^m).
Кривая описывается уравнением
	E: y^2 + xy = x^3 + A x^2 + B.
E(GF(2^m)) -- множество аффинных точек E (решений E в GF(2^m)),
O -- бесконечно удаленная точка.

Поддерживаются координаты Лопеса -- Дахаба (с помощью структуры ec_o) 
и аффинные координаты (прямые функции).

\pre Все указатели, передаваемые в функции, действительны.

\pre Буферы точек не пересекаются с буфером описания кривой.

\safe todo
*******************************************************************************
*/

/*
*******************************************************************************
Создание кривой
*******************************************************************************
*/

/*!	\brief Создание эллиптической кривой в координатах Лопеса -- Дахаба

	Создается описание ec эллиптической кривой в координатах Лопеса -- Дахаба
	над полем f с коэффициентами [f->no]A и [f->no]B.
	\return Признак успеха.
	\pre gf2IsOperable(f) == TRUE.
	\post ec->d == 3.
	\post Буферы ec->order и ec->base подготовлены для ecCreateGroup().
	\keep{ec} ec2CreateLD_keep(f->n).
	\deep{stack} ec2CreateLD_deep(f->n, f->deep).
*/
bool_t ec2CreateLD(
	ec_o* ec,			/*!< [in] описание кривой */
	const qr_o* f,		/*!< [in] базовое поле */
	const octet A[],	/*!< [in] коэффициент A */
	const octet B[],	/*!< [in] коэффициент B */
	void* stack			/*!< [in] вспомогательная память */
);

size_t ec2CreateLD_keep(size_t n);
size_t ec2CreateLD_deep(size_t n, size_t f_deep);

/*
*******************************************************************************
Свойства кривой и группы точек
*******************************************************************************
*/

/*!	\brief Корректное описание эллиптической кривой?

	Проверяется корректность описания ec эллиптической кривой. 
	Описание корректно, если:
	-	ecSeemsValid(ec) == TRUE;
	-	gf2IsValid(ec->f) == TRUE;
	-	A, B \in f;
	-	B \neq 0.
	.
	\return Признак корректности.
	\deep{stack} ec2IsValid_deep(ec->f->n).
*/
bool_t ec2IsValid(
	const ec_o* ec,		/*!< [in] описание кривой */
	void* stack			/*!< [in] вспомогательная память */
);

size_t ec2IsValid_deep(size_t n);

/*!	\brief Описание группы точек эллиптической кривой выглядит корректным?

	Проверяется корректность описания группы точек эллиптической кривой ec.
	Описание корректно, если:
	-	ecIsOperableGroup(ec) == TRUE;
	-	|ec->order * ec->cofactor - (2^m + 1)| <= 2^{m/2 + 1} (границы Хассе);
	-	точка ec->base лежит на ec.
	.
	\pre Описание ec работоспособно.
	\return Признак корректности.
	\remark Не проверяется, что порядок ec->base равняется ec->order.
	\deep{stack} ec2SeemsValidGroup_deep(ec->f->n, ec->f->deep).
*/
bool_t ec2SeemsValidGroup(
	const ec_o* ec,		/*!< [in] описание кривой */
	void* stack			/*!< [in] вспомогательная память */
);

size_t ec2SeemsValidGroup_deep(size_t n, size_t f_deep);

/*!	\brief Надежная группа точек эллиптической кривой?

	Для группы точек кривой ec проверяются следующие условия, определяющие ее
	криптографическую надежность:
	-	ec->order -- простое;
	-	ec->order != 2^m (условие Семаева);
	-	ec->order не делит числа 2^{mi} - 1, i <= mov_threshold (MOV).
	.
	\pre Описание ec (включая описание группы точек) работоспособно.
	\expect Описание ec (включая описание группы точек) корректно.
	\return Признак успеха проверки.
	\deep{stack} ec2IsSafeGroup_deep(ec->f->n).
*/
bool_t ec2IsSafeGroup(
	const ec_o* ec,			/*!< [in] описание кривой */
	size_t mov_threshold,	/*!< [in] MOV-порог */
	void* stack				/*!< [in] вспомогательная память */
);

size_t ec2IsSafeGroup_deep(size_t n);

/*
*******************************************************************************
Арифметика аффинных точек
*******************************************************************************
*/

/*!	\brief Аффинная точка лежит на кривой?

	Проверяется, что аффинная точка [2 * ec->f->n]a лежит на кривой ec.
	\pre Описание ec работоспособно.
	\expect Описание ec корректно.
	\return Признак успеха проверки.
	\deep{stack} ec2IsOnA_deep(ec->f->n, ec->f->deep).
*/
bool_t ec2IsOnA(
	const word a[],		/*!< [in] точка */
	const ec_o* ec,		/*!< [in] описание кривой */
	void* stack			/*!< [in] вспомогательная память */
);

size_t ec2IsOnA_deep(size_t n, size_t f_deep);

/*!	\brief Обратная аффинная точка

	Определяется аффинная точка [2 * ec->f->n]b кривой ec, обратная к аффинной 
	точке [2 * ec->f->n]a.
	\pre Описание ec работоспособно.
	\pre Координаты a лежат в базовом поле.
	\expect Описание ec корректно.
	\expect Точка a лежит на ec.
*/
void ec2NegA(
	word b[],			/*!< [out] обратная точка */
	const word a[],		/*!< [in] точка */
	const ec_o* ec		/*!< [in] описание кривой */
);

/*!	\brief Cложение аффинных точек

	Определяется сумма [2 * ec->f->n]c аффинных точек [2 * ec->f->n]a 
	и [2 * ec->f->n]b кривой ec:
	\code
		c <- a + b.
	\endcode
	\pre Описание ec работоспособно.
	\pre Координаты a и b лежат в базовом поле.
	\expect Описание ec корректно.
	\expect Точки a и b лежат на ec.
	\return TRUE, если сумма является аффинной точкой, и FALSE в противном
	случае.
	\deep{stack} ec2AddAA_deep(ec->f->n, ec->f->deep).
*/
bool_t ec2AddAA(
	word c[],			/*!< [out] сумма */
	const word a[],		/*!< [in] первое слагаемое */
	const word b[],		/*!< [in] второе слагаемое */
	const ec_o* ec,		/*!< [in] описание кривой */
	void* stack			/*!< [in] вспомогательная память */
);

size_t ec2AddAA_deep(size_t n, size_t f_deep);

/*!	\brief Вычитание аффинных точек

	Определяется разность [2 * ec->f->n]c аффинных точек [2 * ec->f->n]a 
	и [2 * ec->f->n]b эллиптической кривой ec:
	\code
		c <- a - b.
	\endcode
	\pre Описание ec работоспособно.
	\pre Координаты a и b лежат в базовом поле.
	\expect Описание ec корректно.
	\expect Точки a и b лежат на ec.
	\return TRUE, если разность является аффинной точкой, и FALSE в противном
	случае (a == b).
	\deep{stack} ec2SubAA_deep(n, ec->f->deep).
*/
bool_t ec2SubAA(
	word c[],			/*!< [out] разность */
	const word a[],		/*!< [in] уменьшаемое */
	const word b[],		/*!< [in] вычитаемое */
	const ec_o* ec,		/*!< [in] описание кривой */
	void* stack			/*!< [in] вспомогательная память */
);

size_t ec2SubAA_deep(size_t n, size_t f_deep);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __BEE2_EC2_H */
