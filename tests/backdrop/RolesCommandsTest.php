<?php
/**
 * @file
 * PHPUnit tests for Bee Roles commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's commands for managing users.
 */
class RolesCommandsTest extends TestCase {

  /**
   * Make sure that the roles command works.
   */
  public function test_roles_command_works() {
    $output = shell_exec('bee roles');
    $this->assertStringContainsString("ADMINISTRATOR: 'administer blocks'", (string) $output);
  }

  /**
   * Make sure that the permissions command works.
   */
  public function test_permissions_command_works() {
    $output = shell_exec('bee permissions');
    $this->assertStringContainsString("BLOCK: 'administer blocks'", (string) $output);
  }

  /**
   * Make sure that the role-create command works.
   */
  public function test_role_create_command_works() {
    $output_user = shell_exec('bee role-create example');
    $this->assertStringContainsString("The 'example' role has been created.", (string) $output_user);
  }

  /**
   * Make sure that the role-add-perm command works.
   */
  public function test_role_add_permission_command_works() {
    $output_user = shell_exec('bee role-add-perm \'administer blocks\' example');
    $this->assertStringContainsString("The 'example' role has the following permissions granted: 'administer blocks'", (string) $output_user);
    $output_user = shell_exec('bee role-add-perm \'administer comment settings\' example');
    $this->assertStringContainsString("The 'example' role has the following permissions granted: 'administer blocks','administer comment settings'", (string) $output_user);
  }

  /**
   * Make sure that the role-remove-perm command works.
   */
  public function test_role_remove_permission_command_works() {
    $output_user = shell_exec('bee role-remove-perm "\'administer blocks\',\'administer comment settings\'" example');
    $this->assertStringContainsString("The 'example' role has the following permissions granted:", (string) $output_user);
  }
  /**
   * Make sure that the role-delete command works.
   */
  public function test_role_delete_permission_command_works() {
    $output_user = shell_exec('bee role-delete example');
    $this->assertStringContainsString("The 'example' role has been deleted.", (string) $output_user);
  }

}
