<?php
/**
 * @file
 * Command(s) for working with node.
 */

/**
 * Implements hook_bee_command().
 */
function node_bee_command() {
  return array(
    'nodes' => array(
      'description' => bt('List all nodes.'),
      'callback' => 'node_bee_callback',
      'group' => 'node',
      'aliases' => array('nls', 'node-list'),
      'options' => array(
        'nid' => array(
          'description' => bt("Get content of a  node."),
          'value' => bt('nid'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee node' => bt('Display a list of all nodese.'),
      ),
    ),
    'node-update' => array(
      'description' => bt('Add or update a node.'),
      'callback' => 'node_update_bee_callback',
      'group' => 'node',
      'options' => array(
        'nid' => array(
          'description' => bt("Give nid of the node to change. (int)"),
          'value' => bt('nid'),
        ),
        'uid' => array(
          'description' => bt("Give uid for the node, add or change. (int)"),
          'value' => bt('uid'),
        ),
        'type' => array(
          'description' => bt("Give node type of the node, add or change. (string)"),
          'value' => bt('type'),
        ),
        'title' => array(
          'description' => bt("Give the title  of the node, add or change. (string)"),
          'value' => bt('title'),
        ),
        'body' => array(
          'description' => bt("Give the text body  of the node, add or change. (string)"),
          'value' => bt('body'),
        ),
        'file' => array(
          'description' => bt("Give the filename to upload text for the body of the node, add or change. (string)"),
          'value' => bt('file'),
        ),
        'status' => array(
          'description' => bt("Publish the node. (0/1)"),
          'value' => bt('status'),
        ),
        'promote' => array(
          'description' => bt("Promote or place to homepage. (0/1)"),
          'value' => bt('promote'),
        ),
        'sticky' => array(
          'description' => bt("Set sticky of the node, add or change. (0/1)"),
          'value' => bt('sticky'),
        ),
      ),
      'aliases' => array('nu'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee node-update --title="This is a new page"' => bt("Add node with title 'This is a new page'."),
        'bee node-update --title="This is a new page" --body="This a test"' => bt("Add node with title 'This is a new page' with the body content 'This a test'."),
        'bee node-update --title="This is a new page" --file="/path/file.html"' => bt("Add node with title 'This is a new page' with the body content from file /path/file.html."),
        'bee node-update --nid=1 --title="This is a updated page"' => bt("Change  node 2 to title 'This is a updated page'."),
      ),
    ),
    'node-delete' => array(
      'description' => bt('Delete a node.'),
      'callback' => 'node_delete_bee_callback',
      'group' => 'node',
      'aliases' => array('ndel'),
      'options' => array(
        'nid' => array(
          'description' => bt("Give nid of the node to delete. (int)"),
          'value' => bt('NID'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee node-delete --nid=3' => bt("Delete node 3."),
      ),
    ),
  );
}

/**
 * Command callback: List all nodes or one.
 */
function node_bee_callback($arguments, $options) {
  if (!empty($options['nid'])) {
    $node = node_load($options['nid']);
    bee_message(bt("The !node node: !title !text", array(
      '!title' => $node->title,
      '!text' => "\n".preg_replace('/\<br(\s*)?\/?\>/i', "\n", $node->body['und'][0]['value']),
      '!node' => $options['nid'],
    )));
  }
  else {
    $rows = array();
    $type = "";
    $nodes = node_load_multiple(FALSE);
    foreach ($nodes as $node) {
      $rows[] = array(
      array('value' => $node->nid),
      array('value' => $node->title),
      array('value' => $node->type),
      array('value' => (user_load($node->uid)->name)),
      array('value' => ($node->status > 0) ? bt('Published') : bt('Not published')),
      array('value' => format_date($node->changed)),
      );
    }

    $header = array(
      array('value' => bt('Nid')),
      array('value' => bt('Title')),
      array('value' => bt('Type')),
      array('value' => bt('Name')),
      array('value' => bt('Status')),
      array('value' => bt('Changed')),
    );
    return array(
      array(
        'type' => 'table',
        'variables' => array(
          'rows' => $rows,
          'header' => $header,
        ),
      ),
    );
  }
}

/**
 * Command callback: Add or update a node
 */
function node_update_bee_callback($arguments, $options) {
  if (!empty($options['type'])) {
    $node_types = array_keys(node_type_get_names());
    if (!in_array($options['type'], $node_types)) {
      $node_types = implode(', ', $node_types);
      bee_message(bt("!type node type name doesn't exists!, types available: !types.", array(
        '!type' => $options['type'],
        '!types' => $node_types,
      )), 'error');
      return;
    }
  }
  if (!empty($options['file'])) {
    if (file_exists($options['file'])) {
      $body = file_get_contents($options['file']);
    }
    else {
      bee_message(bt("!file file doesn't exists!.", array(
        '!file' => $options['file'],
      )), 'error');
      return;
    }
  }
  elseif (!empty($options['body'])) {
    $body = $options['body'];
  }
  else {
    $body = '';
  }

  if (!empty($options['nid'])) {
    $node = node_load($options['nid']);
    if (!empty($node)) {
      $node->uid = !empty($options['uid']) ? $options['uid'] : $node->uid;
      $node->type = !empty($options['type']) ? $options['type'] : $node->type;
      $node->title = !empty($options['title']) ? $options['title'] : $node->title;
      $node->promote = !empty($options['promote']) ? $options['promote'] : $node->promote;
      $node->sticky = !empty($options['sticky']) ? $options['sticky'] : $node->sticky;
      $node->status = !empty($options['status']) ? $options['status'] : $node->status;
      $node->changed = REQUEST_TIME;
      $node->body['und'][0]['value'] = !empty($body) ? $body : $node->body['und'][0]['value'];
      $node->save();
    }
    else {
        bee_message(bt("!nid node doesn't exists!.", array(
          '!nid' => $options['nid'],
        )), 'error');
      return;
    }
  }
  else {
    if (!isset($options['title'])) {
      bee_message(bt("Title option missing."), 'error');
      return;
    }
    $node = entity_create('node', array(
      'uid' => !empty($options['uid']) ? $options['uid'] : 1,
      'type' => !empty($options['type']) ? $options['type'] : 'page',
      'title' => $options['title'],
      'promote' => !empty($options['promote']) ? $options['promote'] : 0,
      'sticky' => !empty($options['sticky']) ? $options['sticky'] : 0,
      'status' => !empty($options['status']) ? $options['status'] : 1,
      'langcode' => LANGUAGE_NONE,
      'created' => REQUEST_TIME,
      'changed' => REQUEST_TIME,
      'body' => array (
        'und' => array (
          0 => array (
            'value' => $body,
            'summary' => !empty($options['summary']) ? $options['summary'] : '',
            'format' => !empty($options['format']) ? $options['format'] : 'filtered_html',
          ),
        ),
      ),
    ));

    $node->save();
    if ( !empty($node->nid)) {
      bee_message(bt("!node node has been created.", array(
        '!node' => $options['title'],
      )), 'success');
    }
    else {
      bee_message(bt("!node node creation failed!", array(
        '!title' => $options['title'],
      )), 'error');
    }
  }
}

/**
 * Command callback: Delete a node
 */
function node_delete_bee_callback($arguments, $options) {
  if (!empty($options['nid'])) {
    $node = node_load($options['nid']);
    if (!$node) {
      bee_message(bt("!nid node doesn't exists!.", array(
        '!nid' => $options['nid'],
      )), 'error');
      return;
    }
      $answer = bee_confirm(bt("Are you sure to delete the node !nid, !title?", array(
        '!nid' => $options['nid'],
        '!title' => $node->title,
      )), FALSE);
    if (!$answer) {
      return;
    }
      node_delete($options['nid']);
      $node = node_load($options['nid']);
    if (empty($node)) {
      bee_message(bt("!nid node is deleted.", array(
        '!nid' => $options['nid'],
      )), 'success');
    }
    else {
      bee_message(bt("!nid node unable to delete!.", array(
        '!nid' => $options['nid'],
      )), 'error');
    }
  }
  else {
      bee_message(bt("!nid node doesn't exists!.", array(
        '!nid' => $options['nid'],
      )), 'error');
    return;
  }
}
