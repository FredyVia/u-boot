/*
 * sun50i H616 DDR3-1333 timings, as programmed by Allwinner's boot0
 *
 * The chips are probably able to be driven by a faster clock, but boot0
 * uses a more conservative timing (as usual).
 *
 * (C) Copyright 2020 Jernej Skrabec <jernej.skrabec@siol.net>
 * Based on H6 DDR3 timings:
 * (C) Copyright 2018,2019 Arm Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>

void mctl_set_timing_params(const struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg *const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	u8 tccd = 4;										/* JEDEC: 4nCK */
	u8 tfaw = ns_to_t(40);					/* JEDEC: 40 ns w/ 1K pages */
	u8 trrd = max(ns_to_t(10), 2);	/* JEDEC: max(10 ns, 2nCK) */
	u8 trcd = max(ns_to_t(18), 2);	/* JEDEC: 13.5 ns */
	u8 trc = ns_to_t(65);						/* JEDEC: 49.5 ns */
	u8 txp = max(ns_to_t(8), 2);		/* JEDEC: max(6 ns, 3nCK) */
	u8 trtp = max(ns_to_t(8), 4);		/* JEDEC: max(7.5 ns, 4nCK) */
	u8 trp = ns_to_t(21);						/* JEDEC: >= 13.75 ns */
	u8 tras = ns_to_t(42);					/* JEDEC >= 36 ns, <= 9*trefi */
	u16 trefi = ns_to_t(3904) / 32; /* JEDEC: 7.8us@Tcase <= 85C */
	u16 trfc = ns_to_t(180);				/* JEDEC: 160 ns for 2Gb */
	u16 txsr = ns_to_t(190);				/* ? */

	u8 tmrw = max(ns_to_t(14), 5);	/* ? */
	u8 tmrd = tmrw;									/* JEDEC: 4nCK */
	u8 tmod = 12;										/* JEDEC: max(15 ns, 12nCK) */
	u8 tcke = max(ns_to_t(15), 2);	/* JEDEC: max(5.625 ns, 3nCK) */
	u8 tcksrx = max(ns_to_t(2), 2); /* JEDEC: max(10 ns, 5nCK) */
	u8 tcksre = max(ns_to_t(5), 2); /* JEDEC: max(10 ns, 5nCK) */
	u8 tckesr = tcke;								/* JEDEC: tCKE(min) + 1nCK */
	u8 trasmax = (trefi * 9) / 32;	/* JEDEC: tREFI * 9 */
	u8 txs = 4;											/* JEDEC: max(5nCK,tRFC+10ns) */
	u8 txsdll = 16;									/* JEDEC: 512 nCK */
	u8 txsabort = 4;								/* ? */
	u8 txsfast = 4;									/* ? */
	u8 tcl = 10;										/* JEDEC: CL / 2 => 6 */
	u8 tcwl = 5;										/* JEDEC: 8 */
	u8 t_rdata_en = 17;							/* ? */
	u8 tphy_wrlat = 5;

	u8 twtp = 24;																/* (WL + BL / 2 + tWR) / 2 */
	u8 twr2rd = max(trrd, (u8)4) + 14;					/* (WL + BL / 2 + tWTR) / 2 */
	u8 trd2wr = (ns_to_t(4) + 17) - ns_to_t(1); /* (RL + BL / 2 + 2 - WL) / 2 */

	/* set DRAM timing */
	writel((twtp << 24) | (tfaw << 16) | (trasmax << 8) | tras,
				 &mctl_ctl->dramtmg[0]);
	writel((txp << 16) | (trtp << 8) | trc, &mctl_ctl->dramtmg[1]);
	writel((tcwl << 24) | (tcl << 16) | (trd2wr << 8) | twr2rd,
				 &mctl_ctl->dramtmg[2]);
	writel((tmrw << 20) | (tmrd << 12) | tmod, &mctl_ctl->dramtmg[3]);
	writel((trcd << 24) | (tccd << 16) | (trrd << 8) | trp,
				 &mctl_ctl->dramtmg[4]);
	writel((tcksrx << 24) | (tcksre << 16) | (tckesr << 8) | tcke,
				 &mctl_ctl->dramtmg[5]);
	/* Value suggested by ZynqMP manual and used by libdram */
	writel((txp + 2) | 0x02020000, &mctl_ctl->dramtmg[6]);
	writel((txsfast << 24) | (txsabort << 16) | (txsdll << 8) | txs,
				 &mctl_ctl->dramtmg[8]);
	writel(0x00020208, &mctl_ctl->dramtmg[9]);
	writel(0xE0C05, &mctl_ctl->dramtmg[10]);
	writel(0x440C021C, &mctl_ctl->dramtmg[11]);
	writel(8, &mctl_ctl->dramtmg[12]);
	writel(0xA100002, &mctl_ctl->dramtmg[13]);
	writel(txsr, &mctl_ctl->dramtmg[14]);

	clrsetbits_le32(&mctl_ctl->init[0], 0xC0000FFF, 0x3f0);
	writel(0x01f20000, &mctl_ctl->init[1]);
	writel(0x00000d05, &mctl_ctl->init[2]);
	writel(0, &mctl_ctl->dfimisc);
	writel(0x0034001b, &mctl_ctl->init[3]);
	writel(0x00330000, &mctl_ctl->init[4]);
	writel(0x00040072, &mctl_ctl->init[6]);
	writel(0x00240009, &mctl_ctl->init[7]);

	clrsetbits_le32(&mctl_ctl->rankctl, 0xff0, 0x660);

	/* Configure DFI timing */
	writel(tphy_wrlat | 0x2000000 | (t_rdata_en << 16) | 0x808000,
				 &mctl_ctl->dfitmg0);
	writel(0x100202, &mctl_ctl->dfitmg1);

	/* set refresh timing */
	writel((trefi << 16) | trfc, &mctl_ctl->rfshtmg);
}
