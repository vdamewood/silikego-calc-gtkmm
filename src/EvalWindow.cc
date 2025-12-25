/* EvalWindow.cc: Expression evaluation window
 * Copyright 2012-2025 Vincent Damewood
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>

#include <gtkmm.h>

#include <SilikegoCore/StringSource.h>
#include <SilikegoCore/InfixParser.h>
#include <SilikegoCore/Value.h>

#include "EvalWindow.h"

static const char UiResource[] = "/com/vdamewood/SilikegoGuiGnome/EvalWindow.ui";

EvalWindow* EvalWindow::Create()
{
	auto builder = Gtk::Builder::create_from_resource(UiResource);
	return Gtk::Builder::get_widget_derived<EvalWindow>(builder, "EvalWindow");
}

EvalWindow::EvalWindow(
		BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder)
	: Gtk::Window(cobject),
	MyBuilder(builder),
	MyButton(builder->get_widget<Gtk::Button>("CalculateButton")),
	MyInput(builder->get_widget<Gtk::Entry>("Input")),
	MyOutput(builder->get_widget<Gtk::Label>("Output"))
{
	Silikego::InstallOperators(MyCaller);
	Silikego::InstallFunctions(MyCaller);
	MyButton->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&EvalWindow::Calculate));
	set_default_widget(*MyButton);
	MyInput->set_activates_default(true);
}

void EvalWindow::Calculate()
{
	Silikego::SyntaxTreeNode ResultTree =
		Silikego::ParseInfix(
		std::unique_ptr<Silikego::DataSource>(new Silikego::StringSource(
			MyInput->get_text().c_str())));
	Silikego::Value Value = ResultTree.evaluate(MyCaller);

	Glib::ustring ResultString;
	switch (Value.status())
	{
	case Silikego::ValueStatus::Integer:
		ResultString = std::to_string(Value.toInteger());
		break;
	case Silikego::ValueStatus::Real:
		ResultString = std::to_string(Value.toReal());
		break;
	case Silikego::ValueStatus::Error:
		switch (Value.toError())
		{
		case Silikego::Error::Memory:
			ResultString = "Memory error";
			break;
		case Silikego::Error::Syntax:
			ResultString = "Syntax error";
			break;
		case Silikego::Error::ZeroDivision:
			ResultString = "Division by zero";
			break;
		case Silikego::Error::FunctionName:
			ResultString = "Function not found";
			break;
		case Silikego::Error::FunctionArguments:
			ResultString = "Bad argument count";
			break;
		case Silikego::Error::Domain:
			ResultString = "Domain error";
			break;
		case Silikego::Error::Range:
			ResultString = "Range error";
			break;
		default:
			ResultString = "Unexpected error";
			break;
			}
	}

	MyOutput->set_text(ResultString);
}
