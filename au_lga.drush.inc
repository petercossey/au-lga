<?php
/**
 * @file
 * Drush commandfile.
 */

/**
 * Implements hook_drush_command().
 */
function au_lga_drush_command() {
  $items = array();
  $items['au-lga-import'] = array(
    'description' => dt('Import Australian LGAs'),
    'aliases' => array('ali'),
  );
  return $items;
}

/**
 * Implements hook_drush_help().
 */
function au_lga_drush_help($section) {
  switch ($section) {
    case 'drush:au-lga-import':
      return dt("Import Australian LGAs.");
  }
}

/**
 * Defines drush batch for recreating activities.
 */
function drush_au_lga_import() {

  $batch = array(
    'operations' => array(
      array('au_lga_import_batch_op', array())
    ),
    'finished' => 'au_lga_import_batch_finished',
    'init_message' => t('Importing states...'),
  );
  batch_set($batch);

  $batch = & batch_get();
  $batch['progressive'] = FALSE;

  drush_backend_batch_process();
}

/**
 * Batch finish callback.
 */
function au_lga_import_batch_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(format_plural(count($results), '1 document is processed.', '@count documents are processed.'));
  }
  else {
    $error_operation = reset($operations);
    drupal_set_message(t('An error occurred while processing @operation', array('@operation' => $error_operation [0])));
  }
}

/**
 * Creates activity as if node was inserted.
 */
function au_lga_import_batch_op(&$context) {
  module_load_include('inc', 'au_lga', 'au_lga.import');
  module_load_include('inc', 'au_lga', 'lib/ShapeFile');
  if (empty($context['sandbox']['progress'])) {
    // Delete old LGAs.
    drush_print(dt('Delete old LGAs'));
    au_lga_delete_lga();

    // Create new LGAs.
    drush_print(dt('Recreate states'));
    $context['sandbox']['states'] = au_lga_import_states();

    // Set initial progress.
    $context['sandbox']['progress'] = 0;

    drush_print(dt('Creating LGAs'));
  }

  // Open data file.
  if (!isset($context['sandbox']['fpos'])) {
    $context['sandbox']['fpos'] = NULL;
  }
  $shp = au_lga_open_data($context['sandbox']['fpos']);

  // Load geoPHP classes.
  geophp_load();

  // Work in cycles by 100.
  $i = 0;
  do {
    // Process node.
    $result = au_lga_create_next_lga($shp, $context['sandbox']['states']);

    // Update our progress information.
    $context['sandbox']['progress']++;
    $i++;

    if ($result != AU_LGA_EOF) {
      $node = $result['node'];
      if (empty($node)) {
        $context['results'][] = 0;
        $msg = 'deleted LGA, skipped';
      }
      else {
        $context['results'][] = $node->nid;
        $msg = $node->title;
      }
      $context['finished'] = 0.5;
      $context['sandbox']['fpos'] = $result['record']->getNextRecordPosition();
    }
    else {
      $msg = 'Finished';
    }
  } while ($result != AU_LGA_EOF && $i < 25);

  $context['message'] = $context['sandbox']['progress'] . ' created (' . $msg . ')';
}
