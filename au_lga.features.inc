<?php
/**
 * @file
 * au_lga.features.inc
 */

/**
 * Implements hook_node_info().
 */
function au_lga_node_info() {
  $items = array(
    'au_lga' => array(
      'name' => t('Australian LGA'),
      'base' => 'node_content',
      'description' => t('Local Government Area of Australia.'),
      'has_title' => '1',
      'title_label' => t('Name'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
