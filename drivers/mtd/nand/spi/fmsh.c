// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 exceet electronics GmbH
 *
 * Authors:
 *	Frieder Schrempf <frieder.schrempf@exceet.de>
 *	Boris Brezillon <boris.brezillon@bootlin.com>
 */

#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/kernel.h>
#endif
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_FMSH		0xA1

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 2, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int fm25s01a_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	return -ERANGE;
}

static int fm25s01a_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 2;
	region->length = 62;

	return 0;
}

static const struct mtd_ooblayout_ops fm25s01a_ooblayout = {
	.ecc = fm25s01a_ooblayout_ecc,
	.rfree = fm25s01a_ooblayout_free,
};

static int fm25s01_ooblayout_ecc(struct mtd_info *mtd, int section,
				 struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 64;

	return 0;
}

static int fm25s01_ooblayout_free(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 2;
	region->length = 62;

	return 0;
}

static const struct mtd_ooblayout_ops fm25s01_ooblayout = {
	.ecc = fm25s01_ooblayout_ecc,
	.rfree = fm25s01_ooblayout_free,
};

static const struct spinand_info fmsh_spinand_table[] = {
	SPINAND_INFO("FM25S01A", 0xE4,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fm25s01a_ooblayout, NULL)),
	SPINAND_INFO("FM25S02A", 0xE5,
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 2, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     1,
		     SPINAND_ECCINFO(&fm25s01a_ooblayout, NULL)),
	SPINAND_INFO("FM25S01", 0xA1,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fm25s01_ooblayout, NULL)),
};

/**
 * fmsh_spinand_detect - initialize device related part in spinand_device
 * struct if it is a FMSH device.
 * @spinand: SPI NAND device structure
 */
static int fmsh_spinand_detect(struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;

	/*
	 * FMSH SPI NAND read ID need a dummy byte,
	 * so the first byte in raw_id is dummy.
	 */
	if (id[1] != SPINAND_MFR_FMSH)
		return 0;

	ret = spinand_match_and_init(spinand, fmsh_spinand_table,
				     ARRAY_SIZE(fmsh_spinand_table), id[2]);
	if (ret)
		return ret;

	return 1;
}

static const struct spinand_manufacturer_ops fmsh_spinand_manuf_ops = {
	.detect = fmsh_spinand_detect,
};

const struct spinand_manufacturer fmsh_spinand_manufacturer = {
	.id = SPINAND_MFR_FMSH,
	.name = "FMSH",
	.ops = &fmsh_spinand_manuf_ops,
};
