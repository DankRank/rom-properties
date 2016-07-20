/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * MegaDriveView.cpp: Sega Mega Drive ROM viewer.                          *
 *                                                                         *
 * Copyright (c) 2016 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "MegaDriveView.hpp"
#include "libromdata/MegaDrive.hpp"
#include <cstdio>

#include "ui_MegaDriveView.h"
class MegaDriveViewPrivate
{
	public:
		MegaDriveViewPrivate(MegaDriveView *q, const LibRomData::MegaDrive *rom);
		~MegaDriveViewPrivate();

	private:
		MegaDriveView *const q_ptr;
		Q_DECLARE_PUBLIC(MegaDriveView)
	private:
		Q_DISABLE_COPY(MegaDriveViewPrivate)

	public:
		Ui::MegaDriveView ui;
		const LibRomData::MegaDrive *rom;

		/**
		 * Update the display widgets.
		 */
		void updateDisplay(void);
};

/** MegaDriveViewPrivate **/

MegaDriveViewPrivate::MegaDriveViewPrivate(MegaDriveView *q, const LibRomData::MegaDrive *rom)
	: q_ptr(q)
	, rom(rom)
{ }

MegaDriveViewPrivate::~MegaDriveViewPrivate()
{
	delete rom;
}

static inline QString rpToQS(const LibRomData::rp_string &rps)
{
#if defined(RP_UTF8)
	return QString::fromUtf8(rps.c_str(), (int)rps.size());
#elif defined(RP_UTF16)
	return QString::fromUtf16(reinterpret_cast<const ushort*>(rps.c_str()), (int)rps.size());
#else
#error Text conversion not available on this system.
#endif
}

/**
 * Update the display widgets.
 */
void MegaDriveViewPrivate::updateDisplay(void)
{
	ui.lblSystem->setText(rpToQS(rom->m_system));
	ui.lblCopyright->setText(rpToQS(rom->m_copyright));
	ui.lblPublisher->setText(rpToQS(rom->m_publisher));
	ui.lblTitleDomestic->setText(rpToQS(rom->m_title_domestic));
	ui.lblTitleExport->setText(rpToQS(rom->m_title_export));
	ui.lblSerialNumber->setText(rpToQS(rom->m_serial));

	// Checksum, in hex.
	char buf[128];
	snprintf(buf, sizeof(buf), "0x%04X", rom->m_checksum);
	ui.lblChecksum->setText(QLatin1String(buf));
	// FIXME: Verify checksum?
	ui.lblChecksumStatus->setVisible(false);

	// I/O support.
	ui.chkIO3btn->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_JOYPAD_3));
	ui.chkIO6btn->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_JOYPAD_6));
	ui.chkIO2btn->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_JOYPAD_SMS));
	ui.chkIOTeamPlayer->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_TEAM_PLAYER));
	ui.chkIOKeyboard->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_KEYBOARD));
	ui.chkIOSerial->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_SERIAL));
	ui.chkIOPrinter->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_PRINTER));
	ui.chkIOTablet->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_TABLET));
	ui.chkIOTrackball->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_TRACKBALL));
	ui.chkIOPaddle->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_PADDLE));
	ui.chkIOFloppy->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_FDD));
	ui.chkIOMegaCD->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_CDROM));
	ui.chkIOActivator->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_ACTIVATOR));
	ui.chkIOMegaMouse->setChecked(!!(rom->m_io_support & LibRomData::MegaDrive::IO_MEGA_MOUSE));

	// ROM range.
	snprintf(buf, sizeof(buf), "0x%08X - 0x%08X", rom->m_rom_start, rom->m_rom_end);
	ui.lblROMRange->setText(QLatin1String(buf));
	// RAM range.
	snprintf(buf, sizeof(buf), "0x%08X - 0x%08X", rom->m_ram_start, rom->m_ram_end);
	ui.lblRAMRange->setText(QLatin1String(buf));

	// TODO: SRAM.

	// Vectors.
	snprintf(buf, sizeof(buf), "0x%08X", rom->m_entry_point);
	ui.lblEntryPoint->setText(QLatin1String(buf));
	snprintf(buf, sizeof(buf), "0x%08X", rom->m_initial_sp);
	ui.lblInitialSP->setText(QLatin1String(buf));
}

/** MegaDriveView **/

MegaDriveView::MegaDriveView(const LibRomData::MegaDrive *rom, QWidget *parent)
	: super(parent)
	, d_ptr(new MegaDriveViewPrivate(this, rom))
{
	Q_D(MegaDriveView);
	d->ui.setupUi(this);

	// Update the display widgets.
	d->updateDisplay();
}

MegaDriveView::~MegaDriveView()
{
	delete d_ptr;
}
