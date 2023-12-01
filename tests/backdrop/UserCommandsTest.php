<?php
/**
 * @file
 * PHPUnit tests for Bee User commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's commands for managing users.
 */
class UserCommandsTest extends TestCase {

  /**
   * Make sure that the users command works.
   */
  public function test_users_command_works() {
    $output = shell_exec('bee users');
    $this->assertRegExp('/| 1 +| admin +| admin@example.com +|/', (string) $output);
  }

  /**
   * Make sure that the user-password command works.
   */
  public function test_user_password_command_works() {
    $output_password = shell_exec('bee user-password admin 123456');
    $this->assertStringContainsString("The password for 'admin' has been reset.", (string) $output_password);
    $this->assertStringNotContainsString('The new password is:', (string) $output_password);

    $output_random = shell_exec('bee user-password admin');
    $this->assertStringContainsString("The password for 'admin' has been reset.", (string) $output_random);
    $this->assertStringContainsString('The new password is:', (string) $output_random);
  }

  /**
   * Make sure that the user-login command works.
   */
  public function test_user_login_command_works() {
    $output = shell_exec('bee user-login admin');
    $this->assertStringContainsString("Use the following link to login as 'admin':", (string) $output);
    $this->assertStringContainsString('/user/reset/1/', (string) $output);

    // Test that leaving the username argument blank loads User 1.
    $output = shell_exec('bee user-login');
    $this->assertStringContainsString("Use the following link to login as 'admin':", (string) $output);
    $this->assertStringContainsString('/user/reset/1/', (string) $output);
  }

  /**
   * Make sure that the user-create command works.
   */
  public function test_user_create_command_works() {
    $output_user = shell_exec('bee user-create joe --mail=joe@example.com --password=P@55w0rd');
    $this->assertStringContainsString("User 'joe' has been created, with password P@55w0rd", (string) $output_user);
  }

  /**
   * Make sure that the user-block command works.
   */
  public function test_user_block_command_works() {
    $output_user = shell_exec('bee user-block joe');
    $this->assertStringContainsString("User 'joe' has been blocked.", (string) $output_user);
  }

  /**
   * Make sure that the user-unblock command works.
   */
  public function test_user_unblock_command_works() {
    $output_user = shell_exec('bee user-unblock joe');
    $this->assertStringContainsString("User 'joe' has been unblocked.", (string) $output_user);
  }

  /**
   * Make sure that the user-add-role command works.
   */
  public function test_user_add_role_command_works() {
    $output_user = shell_exec('bee user-add-role editor joe');
    $this->assertStringContainsString("The 'editor' role has been assigned to user 'joe'.", (string) $output_user);
  }

  /**
   * Make sure that the user-remove-role command works.
   */
  public function test_user_remove_role_command_works() {
    $output_user = shell_exec('bee user-remove-role editor joe');
    $this->assertStringContainsString("The 'editor' role has been removed from user 'joe'.", (string) $output_user);
  }

  /**
   * Make sure that the user-cancel command works.
   */
  public function test_user_cancel_command_works() {
    $output_user = shell_exec('bee user-cancel -y joe');
    $this->assertStringContainsString("User account 'joe' has been removed.", (string) $output_user);
  }
}
