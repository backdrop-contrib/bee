<?php
/**
 * @file
 * PHPUnit tests for Bee Roles commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's commands for managing users.
 */
class NodesCommandsTest extends TestCase {


  /**
   * Make sure that the node-update command works.
   */
  public function test_node_update_command_works() {
    $output = shell_exec('bee node-update --title=\'my new page\'');
    $this->assertStringContainsString("my new page node has been created", (string) $output);
  }
  /**
   * Make sure that the nodes command works.
   */
  public function test_nodes_command_works() {
    $output = shell_exec('bee nodes');
    $this->assertStringContainsString("my new page", (string) $output);
  }

  /**
   * Make sure that the node-delete command works.
   */
  public function test_node_delete_perm_command_works() {
    $output = shell_exec('bee node-delete --nid=1 -y');
    $this->assertStringContainsString("1 node is deleted", (string) $output);
  }

}
