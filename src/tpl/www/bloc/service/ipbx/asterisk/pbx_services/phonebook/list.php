<?php

#
# XiVO Web-Interface
# Copyright (C) 2006-2016  Avencall
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

$url = &$this->get_module('url');
$form = &$this->get_module('form');
$dhtml = &$this->get_module('dhtml');

$pager = $this->get_var('pager');
$act = $this->get_var('act');
$sort = $this->get_var('sort');

$param = array();

if(($search = (string) $this->get_var('search')) !== ''):
	$param['search'] = $search;
else:
	$param = null;
endif;

$page = $url->pager($pager['pages'],
		    $pager['page'],
		    $pager['prev'],
		    $pager['next'],
		    'service/ipbx/pbx_services/phonebook',
		    array('act' => $act,$param));

?>
<div class="b-list">
<?php
	if($page !== ''):
		echo	'<div class="b-total">',
			$this->bbf('number_phonebook-result',
				   '<b>'.$this->get_var('total').'</b>'),
			'</div><div class="b-page">',
			$page,
			'</div><div class="clearboth"></div>';
	endif;
?>
<form action="#" name="fm-phonebook-list" method="post" accept-charset="utf-8">
<?php
	echo	$form->hidden(array('name'	=> DWHO_SESS_NAME,
				    'value'	=> DWHO_SESS_ID)),

		$form->hidden(array('name'	=> 'act',
				    'value'	=> $act)),

		$form->hidden(array('name'	=> 'page',
				    'value'	=> $pager['page'])),

		$form->hidden(array('name'	=> 'search',
				    'value'	=> ''));
?>
<table id="table-main-listing">
	<tr class="sb-top">
		<th class="th-left xspan"><span class="span-left">&nbsp;</span></th>
		<th class="th-center">
			<span class="title <?= $sort[1]=='phonebook'?'underline':''?>">
				<?=$this->bbf('col_phonebook');?>
			</span>
<?php
	echo	$url->href_html(
					$url->img_html('img/updown.png', $this->bbf('col_sort_phonebook'), 'border="0"'),
					'service/ipbx/pbx_services/phonebook',
					array('act'	=> 'list', 'sort' => 'name'),
					null,
					$this->bbf('col_sort_phonebook'));
?>
		<th class="th-center">
			<span class="title <?= $sort[1]=='entity'?'underline':''?>">
				<?=$this->bbf('col_entity');?>
			</span>
<?php
	echo	$url->href_html(
					$url->img_html('img/updown.png', $this->bbf('col_sort_entity'), 'border="0"'),
					'service/ipbx/pbx_services/phonebook',
					array('act'	=> 'list', 'sort' => 'entity'),
					null,
					$this->bbf('col_sort_entity'));
?>
		</th>
		</th>
		<th class="th-right xspan"><span class="span-right">&nbsp;</span></th>
	</tr>
<?php
	if(($list = $this->get_var('list')) === false || ($nb = count($list)) === 0):
?>
	<tr class="sb-content">
		<td colspan="4" class="td-single"><?=$this->bbf('no_phonebook');?></td>
	</tr>
<?php
	else:
		for($i = 0;$i < $nb;$i++):

			$ref = &$list[$i];

?>
	<tr onmouseover="this.tmp = this.className; this.className = 'sb-content l-infos-over';"
	    onmouseout="this.className = this.tmp;"
	    class="sb-content l-infos-<?=(($i % 2) + 1)?>on2">
		<td class="td-left">
			<?=$form->checkbox(array('name'		=> 'phonebook[]',
						 'value'	=> $ref['id'],
						 'label'	=> false,
						 'id'		=> 'it-phonebook-'.$i,
						 'checked'	=> false,
						 'paragraph'	=> false));?>
		</td>
		<td class="txt-left">
		<?php
			echo $url->href_html(
				dwho_trunc($ref['name'],40,'...',false),
				'service/ipbx/pbx_services/phonebook',
				array('act'	=> 'list_contacts',
					  'phonebook' => $ref['id'],
					  'entity' => $ref['entity']));
		?>
		</td>
		<td class="txt-center" title="<?=dwho_alttitle($ref['entity']);?>">
			<label for="it-phonebook-<?=$i?>" id="lb-phonebook-<?=$i?>">
				<?=dwho_htmlen(dwho_trunc($ref['entity'],30,'...',false));?>
			</label>
		</td>
		<td class="td-right xspan"/>
	</tr>
<?php
		endfor;
	endif;
?>
	<tr class="sb-foot">
		<td class="td-left xspan b-nosize"><span class="span-left b-nosize">&nbsp;</span></td>
		<td class="td-center" colspan="2"><span class="b-nosize">&nbsp;</span></td>
		<td class="td-right xspan b-nosize"><span class="span-right b-nosize">&nbsp;</span></td>
	</tr>
</table>
</form>
<?php
	if($page !== ''):
		echo	'<div class="b-total">',
			$this->bbf('number_phonebook-result',
				   '<b>'.$this->get_var('total').'</b>'),
			'</div><div class="b-page">',
			$page,
			'</div><div class="clearboth"></div>';
	endif;
?>
</div>
