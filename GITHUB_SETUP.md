# Push and Publish to GitHub

Run these commands from the project folder.

## 1. Check and commit the project

```powershell
cd "C:\Users\PESINGI KAVYA SRI\Downloads\intilligence"
git status
git add .
git commit -m "Publish project to GitHub"
```

## 2. Create the GitHub repository

Create an empty repository on GitHub named `intilligence`. Do not initialize it
with a README, license, or `.gitignore`.

Then connect it to the local repository:

```powershell
git remote add origin https://github.com/<YOUR_GITHUB_USERNAME>/intilligence.git
git branch -M main
git push -u origin main
```

If the remote already exists, replace it instead:

```powershell
git remote set-url origin https://github.com/<YOUR_GITHUB_USERNAME>/intilligence.git
git push -u origin main
```

## 3. Publish the website with GitHub Pages

1. Open the GitHub repository.
2. Select **Settings**.
3. Select **Pages** under **Code and automation**.
4. Under **Build and deployment**, set **Source** to **Deploy from a branch**.
5. Select the `main` branch and `/ (root)`.
6. Select **Save**.

Your site should be available at:

```text
https://<YOUR_GITHUB_USERNAME>.github.io/intilligence/
```

GitHub Pages may take a few minutes to publish the first deployment.

## 4. Update the project later

```powershell
git add .
git commit -m "Update project"
git push
```
