# LAST TRAIN — repo setup and git workflow

## 1. Token scoping

Use a **fine-grained** PAT, not a classic one.

GitHub → Settings → Developer settings → Personal access tokens → Fine-grained tokens.

- **Resource owner:** your account
- **Repository access:** Only select repositories → the one repo, nothing else
- **Permissions:** Contents → Read and write. Metadata → Read-only (added automatically). Nothing else. Not Actions, not Workflows, not Packages, not Admin.
- **Expiry:** 30 days. Renew rather than extend.

### Rules

- Never paste the token into a chat with any model, including Fable, Claude Code or Claude.
- Never put it in a file, a tracked `.env`, a commit message, or a code comment.
- Never put it in the remote URL (`https://TOKEN@github.com/...`), because that writes it into `.git/config` in plaintext.
- Let the credential helper hold it. It is the only thing that should ever store it.

## 2. Local bootstrap

Run these yourself in a terminal, not through a model.

```bash
mkdir last-train && cd last-train
git init -b main

# store credentials in the OS keychain, not a plaintext file
# macOS:
git config --global credential.helper osxkeychain
# Windows:
git config --global credential.helper manager
# Linux:
git config --global credential.helper libsecret

git remote add origin https://github.com/YOUR_USERNAME/last-train.git
```

On the first `git push`, git prompts for a username and password. Enter your
GitHub username, and paste the **token** as the password. The helper stores it
and never asks again. The token never touches the repo.

Alternative, cleaner if you use the GitHub CLI:

```bash
gh auth login --with-token < /path/to/token.txt
rm /path/to/token.txt
```

Delete that file immediately afterwards.

## 3. Files to create before the first commit

`.gitignore`:

```
node_modules/
dist/
dist-ssr/
.vite/
*.local
.env
.env.*
!.env.example
.DS_Store
*.log
coverage/
android/
ios/
.claude/
```

`.gitattributes`, so binary assets do not get mangled or diffed:

```
* text=auto eol=lf
*.gltf binary
*.glb binary
*.ktx2 binary
*.hdr binary
*.bin binary
*.png binary
*.exr binary
```

`assets/README.md` — record where each asset came from and its licence, as you
download them rather than later.

## 4. Prompt addendum for Fable

Append this to the end of the Phase 1 to 5 prompt in `last-train-brief-v2-3d.md`.
It adds no work, it just makes the output land cleanly in a repo.

```
REPOSITORY CONVENTIONS
The output goes into a git repository, so:
- Assume a standard Vite project root. Include package.json, tsconfig.json,
  vite.config.ts and index.html in Phase 1's file tree and produce them in
  Phase 2.
- Never hardcode secrets, tokens, API keys or absolute local paths anywhere.
  There are no secrets in this project; if you think you need one, stop and ask.
- Reference assets by relative path under /assets only.
- At the end of each phase, and only there, output a single suggested commit
  message in Conventional Commits form, for example
  "feat(render): WebGPU pipeline with post stack and quality presets".
  One line. Do not write any other prose.
- Do not produce a .gitignore, a README, CI workflows, licence files or any other
  repository scaffolding. I handle those.
- Do not attempt any git or network operation. You have no repository access.
  Output code only.
```

## 5. Prompt for Claude Code, run after each Fable gate

Run this in the repo directory. It never sees the token.

```
This is the LAST TRAIN repo. I have just pasted in the output of Fable's
Phase N. Before committing:

1. Verify the working tree. Run `git status` and show me anything untracked or
   modified that I might not expect.
2. Scan every changed file for accidentally committed secrets: tokens, API keys,
   passwords, bearer strings, .env contents, absolute local paths, or anything
   matching ghp_ or github_pat_. Report findings and STOP if you find any. Do
   not commit.
3. Confirm .gitignore covers node_modules, dist, .env and the build cache, and
   that no ignored path is already tracked.
4. Run `npm install` then `npm run build`. If the build fails, fix the failures
   before committing and tell me what you changed.
5. Then stage, commit with a Conventional Commits message describing this phase,
   and push to origin main.

Do not ask me for a token or any credential. Git's credential helper is already
configured; if a push fails on authentication, tell me and stop rather than
trying alternatives. Never write a credential into any file, remote URL or
command you run.

British spelling. Never use em-dashes.
```

## 6. Branch strategy

Simplest thing that still protects you:

- `main` holds working builds only.
- One branch per Fable phase: `phase/02-render`, `phase/03-geometry`, and so on.
- Merge to `main` only after you have approved that phase's gate.
- Tag each merge: `git tag gate-b && git push --tags`.

This matters more here than on a normal project. If Gate B looks wrong and you
fall back to the v1 canvas build, you want a clean tagged point to branch from
rather than an unpickable history.

## 7. What to put in Project Knowledge

Upload all three documents so future chats have the full text:

- `last-train-brief.md` (v1 canvas fallback)
- `last-train-brief-v2-3d.md` (chosen build)
- `last-train-repo-workflow.md` (this file)

Add the repo URL to the project description once it exists.
