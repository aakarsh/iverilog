/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: stub.c,v 1.18 2000/10/15 21:02:08 steve Exp $"
#endif

/*
 * This is a sample target module. All this does is write to the
 * output file some information about each object handle when each of
 * the various object functions is called. This can be used to
 * understand the behavior of the core as it uses a target module.
 */

# include  <ivl_target.h>
# include  <stdio.h>

static FILE*out;

static void show_expression(ivl_expr_t net, unsigned ind)
{
      const ivl_expr_type_t code = ivl_expr_type(net);

      switch (code) {

	  case IVL_EX_NUMBER: {
		const char*bits = ivl_expr_bits(net);
		unsigned width = ivl_expr_width(net);
		unsigned idx;

		fprintf(out, "%*s<number=%u'b", ind, "", width);
		for (idx = width ;  idx > 0 ;  idx -= 1)
		      fprintf(out, "%c", bits[idx-1]);

		fprintf(out, ", %s>\n", ivl_expr_signed(net)? "signed"
			: "unsigned");
		break;
	  }

	  case IVL_EX_STRING:
	    fprintf(out, "%*s<string=\"%s\", width=%u>\n", ind, "",
		    ivl_expr_string(net), ivl_expr_width(net));
	    break;

	  case IVL_EX_SFUNC:
	    fprintf(out, "%*s<function=\"%s\", width=%u>\n", ind, "",
		    ivl_expr_name(net), ivl_expr_width(net));
	    break;

	  case IVL_EX_SIGNAL:
	    fprintf(out, "%*s<signal=%s, width=%u>\n", ind, "",
		    ivl_expr_name(net), ivl_expr_width(net));
	    break;

	  default:
	    fprintf(out, "%*s<expr_type=%u>\n", ind, "", code);
	    break;
      }
}

static void show_statement(ivl_statement_t net, unsigned ind)
{
      const ivl_statement_type_t code = ivl_statement_type(net);

      switch (code) {
	  case IVL_ST_ASSIGN:
	    fprintf(out, "%*sASSIGN <lwidth=%u>\n", ind, "",
		    ivl_stmt_lwidth(net));
	    show_expression(ivl_stmt_rval(net), ind+4);
	    break;

	  case IVL_ST_BLOCK: {
		unsigned cnt = ivl_stmt_block_count(net);
		unsigned idx;
		fprintf(out, "%*sbegin\n", ind, "");
		for (idx = 0 ;  idx < cnt ;  idx += 1) {
		      ivl_statement_t cur = ivl_stmt_block_stmt(net, idx);
		      show_statement(cur, ind+4);
		}
		fprintf(out, "%*send\n", ind, "");
		break;
	  }

	  case IVL_ST_CONDIT: {
		ivl_statement_t t = ivl_stmt_cond_true(net);
		ivl_statement_t f = ivl_stmt_cond_false(net);

		fprintf(out, "%*sif (...)\n", ind, "");
		if (t)
		      show_statement(t, ind+4);
		else
		      fprintf(out, "%*s;\n", ind+4, "");

		if (f) {
		      fprintf(out, "%*selse\n", ind, "");
		      show_statement(f, ind+4);
		}

		break;
	  }

	  case IVL_ST_DELAY:
	    fprintf(out, "%*s#%lu\n", ind, "", ivl_stmt_delay_val(net));
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
	    break;

	  case IVL_ST_NOOP:
	    fprintf(out, "%*s/* noop */;\n", ind, "");
	    break;

	  case IVL_ST_STASK: {
		unsigned idx;
		fprintf(out, "%*sCall %s(%u parameters);\n", ind, "",
			ivl_stmt_name(net), ivl_stmt_parm_count(net));
		for (idx = 0 ;  idx < ivl_stmt_parm_count(net) ;  idx += 1)
		      show_expression(ivl_stmt_parm(net, idx), ind+4);
		break;
	  }

	  case IVL_ST_TRIGGER:
	    fprintf(out, "%*s-> ...\n", ind, "");
	    break;


	  case IVL_ST_WAIT:
	    fprintf(out, "%*s@(...)\n", ind, "");
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
	    break;

	  case IVL_ST_WHILE:
	    fprintf(out, "%*swhile (<?>)\n", ind, "");
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
	    break;

	  default:
	    fprintf(out, "%*sunknown statement type (%u)\n", ind, "", code);
      }
}

static int show_process(ivl_process_t net)
{
      switch (ivl_process_type(net)) {
	  case IVL_PR_INITIAL:
	    fprintf(out, "initial\n");
	    break;
	  case IVL_PR_ALWAYS:
	    fprintf(out, "always\n");
	    break;
      }

      show_statement(ivl_process_stmt(net), 4);

      return 0;
}


static void show_signal(ivl_signal_t net)
{
      const char*type = "?";
      const char*port = "";

      switch (ivl_signal_type(net)) {
	  case IVL_SIT_REG:
	    type = "reg";
	    break;
	  case IVL_SIT_WIRE:
	    type = "wire";
	    break;
      }

      switch (ivl_signal_port(net)) {

	  case IVL_SIP_INPUT:
	    port = "input ";
	    break;

	  case IVL_SIP_OUTPUT:
	    port = "output ";
	    break;

	  case IVL_SIP_INOUT:
	    port = "inout ";
	    break;
      }

      fprintf(out, "  %s %s[%u] %s\n", type, port,
	      ivl_signal_pins(net), ivl_signal_basename(net));
}

static void show_logic(ivl_net_logic_t net)
{
      unsigned npins, idx;
      const char*name = ivl_logic_basename(net);

      switch (ivl_logic_type(net)) {
	  case IVL_LO_AND:
	    fprintf(out, "  and %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_BUF:
	    fprintf(out, "  buf %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_BUFZ:
	    fprintf(out, "  bufz %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_OR:
	    fprintf(out, "  or %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_XOR:
	    fprintf(out, "  xor %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  default:
	    fprintf(out, "  unsupported gate %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    return;
      }

      npins = ivl_logic_pins(net);
      for (idx = 1 ;  idx < npins ;  idx += 1)
	    fprintf(out, ", %s", ivl_nexus_name(ivl_logic_pin(net,idx)));

      fprintf(out, ");\n");
}

static int show_scope(ivl_scope_t net)
{
      unsigned idx;

      fprintf(out, "scope: %s (%u signals, %u logic)\n", ivl_scope_name(net),
	      ivl_scope_sigs(net), ivl_scope_logs(net));

      for (idx = 0 ;  idx < ivl_scope_sigs(net) ;  idx += 1)
	    show_signal(ivl_scope_sig(net, idx));

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1)
	    show_logic(ivl_scope_log(net, idx));

      fprintf(out, "end scope %s\n", ivl_scope_name(net));
      return ivl_scope_children(net, show_scope);
}

int target_start_design(ivl_design_t des)
{
      const char*path = ivl_design_flag(des, "-o");
      if (path == 0) {
	    return -1;
      }

      out = fopen(path, "w");
      if (out == 0) {
	    perror(path);
	    return -2;
      }

      fprintf(out, "root module = %s;\n",
	      ivl_scope_name(ivl_design_root(des)));
      return 0;
}

void target_end_design(ivl_design_t des)
{
      show_scope(ivl_design_root(des));
      ivl_design_process(des, show_process);
      fclose(out);
}

int target_net_const(const char*name, ivl_net_const_t net)
{
      unsigned idx;
      unsigned wid = ivl_const_pins(net);
      const char*bits = ivl_const_bits(net);

      fprintf(out, "LPM_CONSTANT %s: %s%u'b", name,
	      ivl_const_signed(net)? "+- ":"",
	      wid);

      for (idx = 0 ;  idx < wid ;  idx += 1)
	    fprintf(out, "%c", bits[wid-1-idx]);

      fprintf(out, " (%s", ivl_nexus_name(ivl_const_pin(net, 0)));
      for (idx = 1 ;  idx < wid ;  idx += 1)
	    fprintf(out, ", %s", ivl_nexus_name(ivl_const_pin(net, idx)));

      fprintf(out, ")\n");
      return 0;
}

int target_net_event(const char*name, ivl_net_event_t net)
{
      fprintf(out, "STUB: %s: event\n", name);
      return 0;
}

int target_net_probe(const char*name, ivl_net_probe_t net)
{
      fprintf(out, "STUB: %s: probe\n", name);
      return 0;
}

#ifdef __CYGWIN32__
#include <cygwin/cygwin_dll.h>
DECLARE_CYGWIN_DLL(DllMain);
#endif

/*
 * $Log: stub.c,v $
 * Revision 1.18  2000/10/15 21:02:08  steve
 *  Makefile patches to support target loading under cygwin.
 *
 * Revision 1.17  2000/10/15 04:46:23  steve
 *  Scopes and processes are accessible randomly from
 *  the design, and signals and logic are accessible
 *  from scopes. Remove the target calls that are no
 *  longer needed.
 *
 *  Add the ivl_nexus_ptr_t and the means to get at
 *  them from nexus objects.
 *
 *  Give names to methods that manipulate the ivl_design_t
 *  type more consistent names.
 *
 * Revision 1.16  2000/10/08 05:00:04  steve
 *  Missing stream in call to fprintf.
 *
 * Revision 1.15  2000/10/07 19:45:43  steve
 *  Put logic devices into scopes.
 *
 * Revision 1.14  2000/10/06 23:46:51  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.13  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 */

