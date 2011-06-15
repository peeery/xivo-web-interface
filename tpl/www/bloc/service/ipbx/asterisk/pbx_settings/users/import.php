<?php

#
# XiVO Web-Interface
# Copyright (C) 2006-2011  Proformatique <technique@proformatique.com>
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

$form = &$this->get_module('form');
$import_file = $this->get_var('import_file');

?>
<div class="b-infos b-form">
	<h3 class="sb-top xspan">
		<span class="span-left">&nbsp;</span>
		<span class="span-center"><?=$this->bbf('title_content_name');?></span>
		<span class="span-right">&nbsp;</span>
	</h3>
	<div class="sb-content">
        <fieldset id="ipbximportuser-instruction">
        	<?=nl2br($this->bbf('ipbximportuser_instruction'));?>
        <div class="fm-paragraph fm-description">
        	<p>
        		<label id="lb-example" for="it-example"><?=$this->bbf('fm_example');?></label>
        	</p>
        <?php
        echo	$form->textarea(array('paragraph'	=> false,
        			      'label'		=> false,
        			      'notag'		=> false,
        			      'name'		=> 'example',
        			      'id'		=> 'it-example',
        			      'cols'		=> 60,
        			      'rows'		=> 4,
        			      'readonly'	=> true),
        			$this->bbf('fm_example-content'));
        ?>
        </div>
        </fieldset>
		<form action="#" method="post" enctype="multipart/form-data" accept-charset="utf-8">
<?php
		echo	$form->hidden(array('name'	=> 'max_file_size',
					    'value'	=> $import_file['size'])),

			$form->hidden(array('name'	=> DWHO_SESS_NAME,
					    'value'	=> DWHO_SESS_ID)),

			$form->hidden(array('name'	=> 'fm_send',
					    'value'	=> 1)),

			$form->hidden(array('name'	=> 'act',
					    'value'	=> 'import')),

			$form->file(array('desc'	=> $this->bbf('fm_import'),
					  'name'	=> 'import',
					  'labelid'	=> 'import',
					  'size'	=> 15)),

			$form->submit(array('name'	=> 'submit',
					    'id'	=> 'it-submit',
					    'value'	=> $this->bbf('fm_bt-save')));
?>
		</form>
	</div>
	<div class="sb-foot xspan">
		<span class="span-left">&nbsp;</span>
		<span class="span-center">&nbsp;</span>
		<span class="span-right">&nbsp;</span>
	</div>
</div>
